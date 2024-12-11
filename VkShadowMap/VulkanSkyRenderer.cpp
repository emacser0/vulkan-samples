#include "VulkanSkyRenderer.h"
#include "VulkanContext.h"
#include "VulkanHelpers.h"
#include "VulkanTexture.h"
#include "VulkanScene.h"
#include "VulkanSwapchain.h"
#include "VulkanRenderPass.h"

#include "Utils.h"
#include "Engine.h"
#include "Config.h"
#include "Mesh.h"
#include "World.h"

#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"

#include <vector>
#include <array>
#include <algorithm>
#include <execution>

struct FUniformBufferObject
{
	alignas(16) glm::mat4 CameraRotation;
	alignas(16) glm::mat4 Projection;
};

FVulkanSkyRenderer::FVulkanSkyRenderer(FVulkanContext* InContext)
	: FVulkanObject(InContext)
	, DescriptorSetLayout(VK_NULL_HANDLE)
	, Sampler(nullptr)
	, bInitialized(false)
{
	CreateTextureSampler();
	CreateDescriptorSetLayout();
	CreateGraphicsPipelines();
}

FVulkanSkyRenderer::~FVulkanSkyRenderer()
{
	VkDevice Device = Context->GetDevice();

	vkDestroyDescriptorSetLayout(Device, DescriptorSetLayout, nullptr);
}

void FVulkanSkyRenderer::CreateGraphicsPipelines()
{
	VkDevice Device = Context->GetDevice();

	std::string ShaderDirectory;
	GConfig->Get("ShaderDirectory", ShaderDirectory);

	FVulkanShader* VS = Context->CreateObject<FVulkanShader>();
	VS->LoadFile(ShaderDirectory + "sky.vert.spv");
	FVulkanShader* FS = Context->CreateObject<FVulkanShader>();
	FS->LoadFile(ShaderDirectory + "sky.frag.spv");

	Pipeline = Context->CreateObject<FVulkanPipeline>();
	Pipeline->SetVertexShader(VS);
	Pipeline->SetFragmentShader(FS);

	VkPipelineShaderStageCreateInfo VertexShaderStageCI{};
	VertexShaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	VertexShaderStageCI.stage = VK_SHADER_STAGE_VERTEX_BIT;
	VertexShaderStageCI.module = Pipeline->GetVertexShader()->GetModule();
	VertexShaderStageCI.pName = "main";

	VkPipelineShaderStageCreateInfo FragmentShaderStageCI{};
	FragmentShaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	FragmentShaderStageCI.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	FragmentShaderStageCI.module = Pipeline->GetFragmentShader()->GetModule();
	FragmentShaderStageCI.pName = "main";

	std::array<VkPipelineShaderStageCreateInfo, 2> ShaderStageCIs = { VertexShaderStageCI, FragmentShaderStageCI };

	std::vector<VkVertexInputBindingDescription> VertexInputBindingDescs;
	std::vector<VkVertexInputAttributeDescription> VertexInputAttributeDescs;
	GetVertexInputBindings(VertexInputBindingDescs);
	GetVertexInputAttributes(VertexInputAttributeDescs);

	VkPipelineVertexInputStateCreateInfo VertexInputStateCI = Vk::GetVertexInputStateCI(VertexInputBindingDescs, VertexInputAttributeDescs);
	VkPipelineInputAssemblyStateCreateInfo InputAssemblyStateCI = Vk::GetInputAssemblyStateCI();
	VkPipelineViewportStateCreateInfo ViewportStateCI = Vk::GetViewportStateCI();
	VkPipelineRasterizationStateCreateInfo RasterizerCI = Vk::GetRasterizationStateCI();
	RasterizerCI.cullMode = VK_CULL_MODE_NONE;
	VkPipelineMultisampleStateCreateInfo MultisampleStateCI = Vk::GetMultisampleStateCI();
	VkPipelineDepthStencilStateCreateInfo DepthStencilStateCI = Vk::GetDepthStencilStateCI();
	DepthStencilStateCI.depthTestEnable = VK_FALSE;
	DepthStencilStateCI.depthWriteEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState ColorBlendAttachmentState = Vk::GetColorBlendAttachment();
	VkPipelineColorBlendStateCreateInfo ColorBlendStateCI = Vk::GetColorBlendStateCI();
	ColorBlendStateCI.pAttachments = &ColorBlendAttachmentState;

	std::vector<VkDynamicState> DynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	VkPipelineDynamicStateCreateInfo DynamicStateCI{};
	DynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	DynamicStateCI.dynamicStateCount = static_cast<uint32_t>(DynamicStates.size());
	DynamicStateCI.pDynamicStates = DynamicStates.data();

	VkPipelineLayoutCreateInfo PipelineLayoutCI{};
	PipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	PipelineLayoutCI.setLayoutCount = 1;
	PipelineLayoutCI.pSetLayouts = &DescriptorSetLayout;

	Pipeline->CreateLayout(PipelineLayoutCI);

	VkGraphicsPipelineCreateInfo PipelineCI{};
	PipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	PipelineCI.stageCount = static_cast<uint32_t>(ShaderStageCIs.size());
	PipelineCI.pStages = ShaderStageCIs.data();
	PipelineCI.pVertexInputState = &VertexInputStateCI;
	PipelineCI.pInputAssemblyState = &InputAssemblyStateCI;
	PipelineCI.pViewportState = &ViewportStateCI;
	PipelineCI.pRasterizationState = &RasterizerCI;
	PipelineCI.pDepthStencilState = &DepthStencilStateCI;
	PipelineCI.pMultisampleState = &MultisampleStateCI;
	PipelineCI.pColorBlendState = &ColorBlendStateCI;
	PipelineCI.pDynamicState = &DynamicStateCI;
	PipelineCI.layout = Pipeline->GetLayout();
	PipelineCI.renderPass = Context->GetRenderPass()->GetHandle();
	PipelineCI.subpass = 0;
	PipelineCI.basePipelineHandle = VK_NULL_HANDLE;

	Pipeline->CreatePipeline(PipelineCI);
}


void FVulkanSkyRenderer::CreateTextureSampler()
{
	Sampler = Context->CreateObject<FVulkanSampler>();
}

void FVulkanSkyRenderer::CreateDescriptorSetLayout()
{
	VkDevice Device = Context->GetDevice();

	VkDescriptorSetLayoutBinding UBOBinding{};
	UBOBinding.binding = 0;
	UBOBinding.descriptorCount = 1;
	UBOBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	UBOBinding.pImmutableSamplers = nullptr;
	UBOBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding CubemapSamplerBinding{};
	CubemapSamplerBinding.binding = 1;
	CubemapSamplerBinding.descriptorCount = 1;
	CubemapSamplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	CubemapSamplerBinding.pImmutableSamplers = nullptr;
	CubemapSamplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::vector<VkDescriptorSetLayoutBinding> Bindings =
	{
		UBOBinding,
		CubemapSamplerBinding
	};

	VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCI{};
	DescriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	DescriptorSetLayoutCI.bindingCount = static_cast<uint32_t>(Bindings.size());
	DescriptorSetLayoutCI.pBindings = Bindings.data();

	VK_ASSERT(vkCreateDescriptorSetLayout(Device, &DescriptorSetLayoutCI, nullptr, &DescriptorSetLayout));
}

void FVulkanSkyRenderer::CreateDescriptorSets()
{
	VkDevice Device = Context->GetDevice();
	VkDescriptorPool DescriptorPool = Context->GetDescriptorPool();

	const uint32_t MaxConcurrentFrames = Context->GetMaxConcurrentFrames();

	std::vector<VkDescriptorSetLayout> Layouts(MaxConcurrentFrames, DescriptorSetLayout);
	VkDescriptorSetAllocateInfo DescriptorSetAllocInfo{};
	DescriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	DescriptorSetAllocInfo.descriptorPool = DescriptorPool;
	DescriptorSetAllocInfo.descriptorSetCount = static_cast<uint32_t>(MaxConcurrentFrames);
	DescriptorSetAllocInfo.pSetLayouts = Layouts.data();

	DescriptorSets.resize(MaxConcurrentFrames);
	VK_ASSERT(vkAllocateDescriptorSets(Device, &DescriptorSetAllocInfo, DescriptorSets.data()));

	UpdateDescriptorSets();
}

void FVulkanSkyRenderer::CreateUniformBuffers()
{
	VkPhysicalDevice PhysicalDevice = Context->GetPhysicalDevice();
	VkDevice Device = Context->GetDevice();

	VkDeviceSize UniformBufferSize = sizeof(FUniformBufferObject);

	const uint32_t MaxConcurrentFrames = Context->GetMaxConcurrentFrames();

	UniformBuffers.resize(MaxConcurrentFrames);
	for (size_t Idx = 0; Idx < MaxConcurrentFrames; ++Idx)
	{
		UniformBuffers[Idx] = Context->CreateObject<FVulkanBuffer>();
		UniformBuffers[Idx]->SetUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		UniformBuffers[Idx]->SetProperties(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		UniformBuffers[Idx]->Allocate(UniformBufferSize);
		UniformBuffers[Idx]->Map();
	}
}

void FVulkanSkyRenderer::GetVertexInputBindings(std::vector<VkVertexInputBindingDescription>& OutDescs)
{
	OutDescs.resize(1);

	OutDescs[0].binding = 0;
	OutDescs[0].stride = sizeof(FVertex);
	OutDescs[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
}

void FVulkanSkyRenderer::GetVertexInputAttributes(std::vector<VkVertexInputAttributeDescription>& OutDescs)
{
	OutDescs.resize(4);

	OutDescs[0].binding = 0;
	OutDescs[0].location = 0;
	OutDescs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	OutDescs[0].offset = offsetof(FVertex, Position);

	OutDescs[1].binding = 0;
	OutDescs[1].location = 1;
	OutDescs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	OutDescs[1].offset = offsetof(FVertex, Normal);

	OutDescs[2].binding = 0;
	OutDescs[2].location = 2;
	OutDescs[2].format = VK_FORMAT_R32G32_SFLOAT;
	OutDescs[2].offset = offsetof(FVertex, TexCoords);

	OutDescs[3].binding = 0;
	OutDescs[3].location = 3;
	OutDescs[3].format = VK_FORMAT_R32G32B32_SFLOAT;
	OutDescs[3].offset = offsetof(FVertex, Tangent);
}

void FVulkanSkyRenderer::UpdateDescriptorSets()
{
	VkDevice Device = Context->GetDevice();

	FVulkanMesh* SkyMesh = GetSkyMesh();
	if (SkyMesh == nullptr)
	{
		return;
	}

	FVulkanMaterial* Material = SkyMesh->GetMaterial();
	if (Material == nullptr)
	{
		return;
	}

	UTexture* TextureAsset = Material->GetBaseColor().TexParam;
	if (TextureAsset == nullptr)
	{
		return;
	}

	FVulkanTexture* Texture = TextureAsset->GetRenderTexture();
	if (Texture == nullptr)
	{
		return;
	}

	for (int32_t Idx = 0; Idx < DescriptorSets.size(); ++Idx)
	{
		std::array<VkWriteDescriptorSet, 2> DescriptorWrites{};

		VkDescriptorBufferInfo UniformBufferInfo{};
		UniformBufferInfo.buffer = UniformBuffers[Idx]->GetHandle();
		UniformBufferInfo.offset = 0;
		UniformBufferInfo.range = sizeof(FUniformBufferObject);

		DescriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		DescriptorWrites[0].dstSet = DescriptorSets[Idx];
		DescriptorWrites[0].dstBinding = 0;
		DescriptorWrites[0].dstArrayElement = 0;
		DescriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		DescriptorWrites[0].descriptorCount = 1;
		DescriptorWrites[0].pBufferInfo = &UniformBufferInfo;

		VkDescriptorImageInfo CubemapImageInfo{};
		CubemapImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		CubemapImageInfo.imageView = Texture->GetImage()->GetView();
		CubemapImageInfo.sampler = Sampler->GetSampler();

		DescriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		DescriptorWrites[1].dstSet = DescriptorSets[Idx];
		DescriptorWrites[1].dstBinding = 1;
		DescriptorWrites[1].dstArrayElement = 0;
		DescriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		DescriptorWrites[1].descriptorCount = 1;
		DescriptorWrites[1].pImageInfo = &CubemapImageInfo;

		vkUpdateDescriptorSets(Device, static_cast<uint32_t>(DescriptorWrites.size()), DescriptorWrites.data(), 0, nullptr);
	}
}

void FVulkanSkyRenderer::UpdateUniformBuffer()
{
	FWorld* World = GEngine->GetWorld();
	if (World == nullptr)
	{
		return;
	}

	FVulkanScene* Scene = World->GetRenderScene();
	if (Scene == nullptr)
	{
		return;
	}

	FVulkanCamera Camera = Scene->GetCamera();

	VkExtent2D SwapchainExtent = Context->GetSwapchain()->GetExtent();

	float FOVRadians = glm::radians(Camera.FOV);
	float AspectRatio = SwapchainExtent.width / (float)SwapchainExtent.height;

	static const glm::mat4 IdentityMatrix(1.0f);

	FUniformBufferObject UBO{};
	UBO.CameraRotation = glm::inverse(glm::toMat4(Camera.Rotation));
	UBO.Projection = glm::perspective(FOVRadians, AspectRatio, Camera.Near, Camera.Far);

	memcpy(UniformBuffers[Context->GetCurrentFrame()]->GetMappedAddress(), &UBO, sizeof(FUniformBufferObject));
}

FVulkanMesh* FVulkanSkyRenderer::GetSkyMesh() const
{
	FWorld* World = GEngine->GetWorld();
	if (World == nullptr)
	{
		return nullptr;
	}

	FVulkanScene* Scene = World->GetRenderScene();
	if (Scene == nullptr)
	{
		return nullptr;
	}

	FVulkanModel* Sky = Scene->GetSky();
	if (Sky == nullptr)
	{
		return nullptr;
	}

	return static_cast<FVulkanMesh*>(Sky->GetMesh());
}

void FVulkanSkyRenderer::PreRender()
{	
	if (bInitialized)
	{
		return;
	}

	CreateUniformBuffers();
	CreateDescriptorSets();

	bInitialized = true;
}

void FVulkanSkyRenderer::Render()
{	
	uint32_t CurrentFrame = Context->GetCurrentFrame();
	VkCommandBuffer CommandBuffer = Context->GetCommandBuffer();

	VkExtent2D SwapchainExtent = Context->GetSwapchain()->GetExtent();

	VkViewport Viewport{};
	Viewport.x = 0.0f;
	Viewport.y = 0.0f;
	Viewport.width = (float)SwapchainExtent.width;
	Viewport.height = (float)SwapchainExtent.height;
	Viewport.minDepth = 0.0f;
	Viewport.maxDepth = 1.0f;

	VkRect2D Scissor{};
	Scissor.offset = { 0, 0 };
	Scissor.extent = SwapchainExtent;

	UpdateUniformBuffer();

	vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline->GetPipeline());

	vkCmdSetViewport(CommandBuffer, 0, 1, &Viewport);
	vkCmdSetScissor(CommandBuffer, 0, 1, &Scissor);

	FVulkanMesh* SkyMesh = GetSkyMesh();
	if (SkyMesh == nullptr)
	{
		return;
	}

	VkDescriptorSet DescriptorSet = DescriptorSets[CurrentFrame];

	vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline->GetLayout(), 0, 1, &DescriptorSet, 0, nullptr);

	VkBuffer VertexBuffers[] = { SkyMesh->GetVertexBuffer()->GetHandle() };
	VkDeviceSize Offsets[] = { 0 };
	vkCmdBindVertexBuffers(CommandBuffer, 0, 1, VertexBuffers, Offsets);

	vkCmdBindIndexBuffer(CommandBuffer, SkyMesh->GetIndexBuffer()->GetHandle(), 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(CommandBuffer, static_cast<uint32_t>(SkyMesh->GetMeshAsset()->GetIndices().size()), 1, 0, 0, 0);
}

