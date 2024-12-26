#include "VulkanMeshRenderer.h"
#include "VulkanContext.h"
#include "VulkanHelpers.h"
#include "VulkanTexture.h"
#include "VulkanScene.h"

#include "Utils.h"
#include "Engine.h"
#include "Config.h"
#include "Camera.h"

#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"

#include <vector>
#include <array>
#include <iostream>
#include <stdexcept>
#include <algorithm>

struct FUniformBufferObject
{
	alignas(16) glm::mat4 Model;
	alignas(16) glm::mat4 View;
	alignas(16) glm::mat4 Projection;
};

FVulkanMeshRenderer::FVulkanMeshRenderer(FVulkanContext* InContext)
	: FVulkanObject(InContext)
	, PipelineLayout(VK_NULL_HANDLE)
	, Pipeline(VK_NULL_HANDLE)
	, DescriptorSetLayout(VK_NULL_HANDLE)
	, TextureSampler(VK_NULL_HANDLE)
	, VertexShader(nullptr)
	, FragmentShader(nullptr)
{
	VertexShader = new FVulkanShader(Context);
	FragmentShader = new FVulkanShader(Context);
}

FVulkanMeshRenderer::~FVulkanMeshRenderer()
{
	VkDevice Device = Context->GetDevice();

	vkDestroyPipelineLayout(Device, PipelineLayout, nullptr);
	vkDestroyPipeline(Device, Pipeline, nullptr);

	vkDestroySampler(Device, TextureSampler, nullptr);

	vkDestroyDescriptorSetLayout(Device, DescriptorSetLayout, nullptr);

	for (const auto& Pair : UniformBufferMap)
	{
		const std::vector<FVulkanBuffer>& UniformBuffers = Pair.second;
		for (const FVulkanBuffer& UniformBuffer : UniformBuffers)
		{
			if (UniformBuffer.Buffer != VK_NULL_HANDLE)
			{
				vkDestroyBuffer(Device, UniformBuffer.Buffer, nullptr);
			}

			if (UniformBuffer.Memory != VK_NULL_HANDLE)
			{
				vkFreeMemory(Device, UniformBuffer.Memory, nullptr);
			}
		}
	}

	delete VertexShader;
	delete FragmentShader;
}

void FVulkanMeshRenderer::Ready()
{	
	CreateDescriptorSetLayout();
	CreateGraphicsPipeline();
	CreateTextureSampler();
	CreateUniformBuffers();
	CreateDescriptorSets();
}

void FVulkanMeshRenderer::CreateDescriptorSetLayout()
{
	VkDevice Device = Context->GetDevice();

	VkDescriptorSetLayoutBinding UBOLayoutBinding{};
	UBOLayoutBinding.binding = 0;
	UBOLayoutBinding.descriptorCount = 1;
	UBOLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	UBOLayoutBinding.pImmutableSamplers = nullptr;
	UBOLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding SamplerLayoutBinding{};
	SamplerLayoutBinding.binding = 1;
	SamplerLayoutBinding.descriptorCount = 1;
	SamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	SamplerLayoutBinding.pImmutableSamplers = nullptr;
	SamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::vector<VkDescriptorSetLayoutBinding> Bindings =
	{
		UBOLayoutBinding,
		SamplerLayoutBinding
	};

	VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCI{};
	DescriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	DescriptorSetLayoutCI.bindingCount = static_cast<uint32_t>(Bindings.size());
	DescriptorSetLayoutCI.pBindings = Bindings.data();

	VK_ASSERT(vkCreateDescriptorSetLayout(Device, &DescriptorSetLayoutCI, nullptr, &DescriptorSetLayout));
}

void FVulkanMeshRenderer::CreateGraphicsPipeline()
{
	VkDevice Device = Context->GetDevice();

	std::string ShaderDirectory;
	GConfig->Get("ShaderDirectory", ShaderDirectory);

	assert(VertexShader->LoadFile(ShaderDirectory + "main.vert.spv"));
	assert(FragmentShader->LoadFile(ShaderDirectory + "main.frag.spv"));

	VkPipelineShaderStageCreateInfo VertexShaderStageCI{};
	VertexShaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	VertexShaderStageCI.stage = VK_SHADER_STAGE_VERTEX_BIT;
	VertexShaderStageCI.module = VertexShader->GetModule();
	VertexShaderStageCI.pName = "main";

	VkPipelineShaderStageCreateInfo FragmentShaderStageCI{};
	FragmentShaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	FragmentShaderStageCI.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	FragmentShaderStageCI.module = FragmentShader->GetModule();
	FragmentShaderStageCI.pName = "main";

	VkPipelineShaderStageCreateInfo ShaderStageCIs[] =
	{
		VertexShaderStageCI,
		FragmentShaderStageCI
	};

	VkVertexInputBindingDescription VertexInputBindingDesc{};
	VertexInputBindingDesc.binding = 0;
	VertexInputBindingDesc.stride = sizeof(FVertex);
	VertexInputBindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputAttributeDescription VertexInputAttributeDescs[3]{};
	VertexInputAttributeDescs[0].binding = 0;
	VertexInputAttributeDescs[0].location = 0;
	VertexInputAttributeDescs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	VertexInputAttributeDescs[0].offset = offsetof(FVertex, Position);

	VertexInputAttributeDescs[1].binding = 0;
	VertexInputAttributeDescs[1].location = 1;
	VertexInputAttributeDescs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	VertexInputAttributeDescs[1].offset = offsetof(FVertex, Color);

	VertexInputAttributeDescs[2].binding = 0;
	VertexInputAttributeDescs[2].location = 2;
	VertexInputAttributeDescs[2].format = VK_FORMAT_R32G32_SFLOAT;
	VertexInputAttributeDescs[2].offset = offsetof(FVertex, TexCoords);

	VkPipelineVertexInputStateCreateInfo VertexInputStateCI{};
	VertexInputStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	VertexInputStateCI.vertexBindingDescriptionCount = 1U;
	VertexInputStateCI.pVertexBindingDescriptions = &VertexInputBindingDesc;
	VertexInputStateCI.vertexAttributeDescriptionCount = 3U;
	VertexInputStateCI.pVertexAttributeDescriptions = VertexInputAttributeDescs;

	VkPipelineInputAssemblyStateCreateInfo InputAssemblyStateCI{};
	InputAssemblyStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	InputAssemblyStateCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	InputAssemblyStateCI.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo ViewportStateCI{};
	ViewportStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	ViewportStateCI.viewportCount = 1;
	ViewportStateCI.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo RasterizerCI{};
	RasterizerCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	RasterizerCI.depthClampEnable = VK_FALSE;
	RasterizerCI.rasterizerDiscardEnable = VK_FALSE;
	RasterizerCI.polygonMode = VK_POLYGON_MODE_FILL;
	RasterizerCI.lineWidth = 1.0f;
	RasterizerCI.cullMode = VK_CULL_MODE_BACK_BIT;
	RasterizerCI.frontFace = VK_FRONT_FACE_CLOCKWISE;
	RasterizerCI.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo MultisampleStateCI{};
	MultisampleStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	MultisampleStateCI.sampleShadingEnable = VK_FALSE;
	MultisampleStateCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineDepthStencilStateCreateInfo DepthStencilStateCI{};
	DepthStencilStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	DepthStencilStateCI.depthTestEnable = VK_TRUE;
	DepthStencilStateCI.depthWriteEnable = VK_TRUE;
	DepthStencilStateCI.depthCompareOp = VK_COMPARE_OP_LESS;
	DepthStencilStateCI.depthBoundsTestEnable = VK_FALSE;
	DepthStencilStateCI.stencilTestEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState ColorBlendAttachmentState{};
	ColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	ColorBlendAttachmentState.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo ColorBlendStateCI{};
	ColorBlendStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	ColorBlendStateCI.logicOpEnable = VK_FALSE;
	ColorBlendStateCI.logicOp = VK_LOGIC_OP_COPY;
	ColorBlendStateCI.attachmentCount = 1;
	ColorBlendStateCI.pAttachments = &ColorBlendAttachmentState;
	ColorBlendStateCI.blendConstants[0] = 0.0f;
	ColorBlendStateCI.blendConstants[1] = 0.0f;
	ColorBlendStateCI.blendConstants[2] = 0.0f;
	ColorBlendStateCI.blendConstants[3] = 0.0f;

	std::vector<VkDynamicState> DynamicStates =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo DynamicStateCI{};
	DynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	DynamicStateCI.dynamicStateCount = static_cast<uint32_t>(DynamicStates.size());
	DynamicStateCI.pDynamicStates = DynamicStates.data();

	VkPipelineLayoutCreateInfo PipelineLayoutCI{};
	PipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	PipelineLayoutCI.setLayoutCount = 1;
	PipelineLayoutCI.pSetLayouts = &DescriptorSetLayout;

	VK_ASSERT(vkCreatePipelineLayout(Device, &PipelineLayoutCI, nullptr, &PipelineLayout))

	VkGraphicsPipelineCreateInfo PipelineCI{};
	PipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	PipelineCI.stageCount = 2;
	PipelineCI.pStages = ShaderStageCIs;
	PipelineCI.pVertexInputState = &VertexInputStateCI;
	PipelineCI.pInputAssemblyState = &InputAssemblyStateCI;
	PipelineCI.pViewportState = &ViewportStateCI;
	PipelineCI.pRasterizationState = &RasterizerCI;
	PipelineCI.pDepthStencilState = &DepthStencilStateCI;
	PipelineCI.pMultisampleState = &MultisampleStateCI;
	PipelineCI.pColorBlendState = &ColorBlendStateCI;
	PipelineCI.pDynamicState = &DynamicStateCI;
	PipelineCI.layout = PipelineLayout;
	PipelineCI.renderPass = Context->GetRenderPass();
	PipelineCI.subpass = 0;
	PipelineCI.basePipelineHandle = VK_NULL_HANDLE;

	VK_ASSERT(vkCreateGraphicsPipelines(Device, VK_NULL_HANDLE, 1, &PipelineCI, nullptr, &Pipeline))
}

void FVulkanMeshRenderer::CreateTextureSampler()
{
	VkPhysicalDevice PhysicalDevice = Context->GetPhysicalDevice();
	VkDevice Device = Context->GetDevice();

	VkPhysicalDeviceProperties Properties{};
	vkGetPhysicalDeviceProperties(PhysicalDevice, &Properties);

	VkSamplerCreateInfo SamplerCI{};
	SamplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	SamplerCI.magFilter = VK_FILTER_LINEAR;
	SamplerCI.minFilter = VK_FILTER_LINEAR;
	SamplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	SamplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	SamplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	SamplerCI.anisotropyEnable = VK_TRUE;
	SamplerCI.maxAnisotropy = Properties.limits.maxSamplerAnisotropy;
	SamplerCI.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	SamplerCI.unnormalizedCoordinates = VK_FALSE;
	SamplerCI.compareEnable = VK_FALSE;
	SamplerCI.compareOp = VK_COMPARE_OP_ALWAYS;
	SamplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	VK_ASSERT(vkCreateSampler(Device, &SamplerCI, nullptr, &TextureSampler))
}

void FVulkanMeshRenderer::CreateUniformBuffers()
{
	VkPhysicalDevice PhysicalDevice = Context->GetPhysicalDevice();
	VkDevice Device = Context->GetDevice();

	VkDeviceSize UniformBufferSize = sizeof(FUniformBufferObject);

	FVulkanScene* Scene = GEngine->GetScene();
	if (Scene == nullptr)
	{
		return;
	}

	UniformBufferMap.clear();
	for (FVulkanModel* Model : Scene->GetModels())
	{
		if (Model == nullptr)
		{
			continue;
		}

		std::vector<FVulkanBuffer> UniformBuffers(MAX_CONCURRENT_FRAME);
		for (size_t Idx = 0; Idx < MAX_CONCURRENT_FRAME; ++Idx)
		{
			Vk::CreateBuffer(
				PhysicalDevice,
				Device,
				UniformBufferSize,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				UniformBuffers[Idx].Buffer,
				UniformBuffers[Idx].Memory);

			vkMapMemory(Device, UniformBuffers[Idx].Memory, 0, UniformBufferSize, 0, &UniformBuffers[Idx].Mapped);
		}

		UniformBufferMap[Model] = UniformBuffers;
	}
}

void FVulkanMeshRenderer::CreateDescriptorSets()
{
	VkDevice Device = Context->GetDevice();
	VkDescriptorPool DescriptorPool = Context->GetDescriptorPool();

	DescriptorSetMap.clear();
	for (FVulkanModel* Model : GEngine->GetScene()->GetModels())
	{
		FVulkanMesh* Mesh = Model->GetMesh();
		if (Mesh == nullptr)
		{
			continue;
		}

		std::vector<VkDescriptorSetLayout> Layouts(MAX_CONCURRENT_FRAME, DescriptorSetLayout);
		VkDescriptorSetAllocateInfo DescriptorSetAllocInfo{};
		DescriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		DescriptorSetAllocInfo.descriptorPool = DescriptorPool;
		DescriptorSetAllocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_CONCURRENT_FRAME);
		DescriptorSetAllocInfo.pSetLayouts = Layouts.data();

		std::vector<VkDescriptorSet> DescriptorSets(MAX_CONCURRENT_FRAME);
		VK_ASSERT(vkAllocateDescriptorSets(Device, &DescriptorSetAllocInfo, DescriptorSets.data()));

		DescriptorSetMap[Model] = DescriptorSets;
	}

	UpdateDescriptorSets();
}

void FVulkanMeshRenderer::WaitIdle()
{
	VkDevice Device = Context->GetDevice();

	vkDeviceWaitIdle(Device);
}

void FVulkanMeshRenderer::UpdateUniformBuffer(FVulkanModel* InModel)
{
	if (InModel == nullptr)
	{
		return;
	}

	FCamera* Camera = GEngine->GetCamera();
	assert(Camera != nullptr);

	VkExtent2D SwapchainExtent = Context->GetSwapchainExtent();

	float FOVRadians = glm::radians(Camera->GetFOV());
	float AspectRatio = SwapchainExtent.width / (float)SwapchainExtent.height;

	static const glm::mat4 IdentityMatrix(1.0f);

	FTransform ModelTransform = InModel->GetTransform();

	FUniformBufferObject UBO{};
	UBO.Model = glm::translate(IdentityMatrix, ModelTransform.GetTranslation()) * glm::toMat4(ModelTransform.GetRotation()) * glm::scale(IdentityMatrix, ModelTransform.GetScale());
	UBO.View = Camera->GetViewMatrix();
	UBO.Projection = glm::perspective(FOVRadians, AspectRatio, 0.1f, 100.0f);

	memcpy(UniformBufferMap[InModel][Context->GetCurrentFrame()].Mapped, &UBO, sizeof(FUniformBufferObject));
}

void FVulkanMeshRenderer::UpdateDescriptorSets()
{
	VkDevice Device = Context->GetDevice();

	for (const auto& Pair : DescriptorSetMap)
	{
		FVulkanModel* Model = Pair.first;
		FVulkanMesh* Mesh = Model->GetMesh();
		if (Mesh == nullptr)
		{
			continue;
		}

		FVulkanTexture* Texture = Mesh->GetTexture();
		if (Texture == nullptr)
		{
			continue;
		}

		const std::vector<VkDescriptorSet>& DescriptorSets = Pair.second;

		for (int32_t Idx = 0; Idx < DescriptorSets.size(); ++Idx)
		{
			std::array<VkWriteDescriptorSet, 2> DescriptorWrites{};

			VkDescriptorBufferInfo BufferInfo{};
			BufferInfo.buffer = UniformBufferMap[Model][Idx].Buffer;
			BufferInfo.offset = 0;
			BufferInfo.range = sizeof(FUniformBufferObject);

			DescriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			DescriptorWrites[0].dstSet = DescriptorSets[Idx];
			DescriptorWrites[0].dstBinding = 0;
			DescriptorWrites[0].dstArrayElement = 0;
			DescriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			DescriptorWrites[0].descriptorCount = 1;
			DescriptorWrites[0].pBufferInfo = &BufferInfo;

			VkDescriptorImageInfo ImageInfo{};
			ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			ImageInfo.imageView = Texture->GetView();
			ImageInfo.sampler = TextureSampler;

			DescriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			DescriptorWrites[1].dstSet = DescriptorSets[Idx];
			DescriptorWrites[1].dstBinding = 1;
			DescriptorWrites[1].dstArrayElement = 0;
			DescriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			DescriptorWrites[1].descriptorCount = 1;
			DescriptorWrites[1].pImageInfo = &ImageInfo;

			vkUpdateDescriptorSets(Device, static_cast<uint32_t>(DescriptorWrites.size()), DescriptorWrites.data(), 0, nullptr);
		}
	}
}

void FVulkanMeshRenderer::Render()
{
	uint32_t CurrentFrame = Context->GetCurrentFrame();
	uint32_t CurrentImageIndex = Context->GetCurrentImageIndex();

	const std::vector<VkCommandBuffer>& CommandBuffers = Context->GetCommandBuffers();
	VkCommandBuffer CommandBuffer = CommandBuffers[CurrentFrame];

	VkExtent2D SwapchainExtent = Context->GetSwapchainExtent();

	vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline);

	VkViewport Viewport{};
	Viewport.x = 0.0f;
	Viewport.y = 0.0f;
	Viewport.width = (float)SwapchainExtent.width;
	Viewport.height = (float)SwapchainExtent.height;
	Viewport.minDepth = 0.0f;
	Viewport.maxDepth = 1.0f;
	vkCmdSetViewport(CommandBuffer, 0, 1, &Viewport);

	VkRect2D Scissor{};
	Scissor.offset = { 0, 0 };
	Scissor.extent = SwapchainExtent;
	vkCmdSetScissor(CommandBuffer, 0, 1, &Scissor);

	for (FVulkanModel* Model : GEngine->GetScene()->GetModels())
	{
		FVulkanMesh* Mesh = Model->GetMesh();
		if (Mesh == nullptr)
		{
			continue;
		}

		const auto DescriptorSetIter = DescriptorSetMap.find(Model);
		if (DescriptorSetIter == DescriptorSetMap.end())
		{
			continue;
		}

		UpdateUniformBuffer(Model);

		vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineLayout, 0, 1, &(DescriptorSetIter->second[CurrentFrame]), 0, nullptr);

		VkBuffer VertexBuffers[] = { Mesh->GetVertexBuffer().Buffer};
		VkDeviceSize Offsets[] = { 0 };
		vkCmdBindVertexBuffers(CommandBuffer, 0, 1, VertexBuffers, Offsets);

		vkCmdBindIndexBuffer(CommandBuffer, Mesh->GetIndexBuffer().Buffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(CommandBuffer, static_cast<uint32_t>(Mesh->GetIndices().size()), 1, 0, 0, 0);
	}
}
