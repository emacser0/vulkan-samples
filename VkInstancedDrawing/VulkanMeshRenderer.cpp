#include "VulkanMeshRenderer.h"
#include "VulkanContext.h"
#include "VulkanHelpers.h"
#include "VulkanTexture.h"
#include "VulkanScene.h"
#include "VulkanLight.h"

#include "Utils.h"
#include "Config.h"
#include "Mesh.h"

#include "Engine.h"

#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"

#include <vector>
#include <array>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <execution>
#include <unordered_map>

struct FUniformBufferObject
{
	alignas(16) glm::mat4 View;
	alignas(16) glm::mat4 Projection;
	alignas(16) glm::vec3 CameraPosition;

	FVulkanLight Light;
};

struct FInstanceBuffer
{
	alignas(16) glm::mat4 Model;
	alignas(16) glm::mat4 ModelView;
	alignas(16) glm::mat4 NormalMatrix;
};

FVulkanMeshRenderer::FVulkanMeshRenderer(FVulkanContext* InContext)
	: FVulkanObject(InContext)
	, DescriptorSetLayout(VK_NULL_HANDLE)
	, TextureSampler(VK_NULL_HANDLE)
{
	GatherInstancedDrawingInfo();

	CreateDescriptorSetLayout();
	CreateGraphicsPipelines();
	CreateTextureSampler();
	CreateUniformBuffers();
	CreateInstanceBuffers();
	CreateDescriptorSets();
}

FVulkanMeshRenderer::~FVulkanMeshRenderer()
{
	VkDevice Device = Context->GetDevice();

	vkDestroyDescriptorSetLayout(Device, DescriptorSetLayout, nullptr);

	vkDestroySampler(Device, TextureSampler, nullptr);

	for (FVulkanBuffer UniformBuffer : UniformBuffers)
	{
		vkFreeMemory(Device, UniformBuffer.Memory, nullptr);
		vkDestroyBuffer(Device, UniformBuffer.Buffer, nullptr);
	}

	for (const auto& Pair : InstancedDrawingMap)
	{
		FVulkanMesh* Mesh = Pair.first;
		const std::vector<FVulkanModel*>& Models = Pair.second.Models;
		const std::vector<FVulkanBuffer>& InstanceBuffers = Pair.second.InstanceBuffers;

		for (const FVulkanBuffer& InstanceBuffer : InstanceBuffers)
		{
			vkFreeMemory(Device, InstanceBuffer.Memory, nullptr);
			vkDestroyBuffer(Device, InstanceBuffer.Buffer, nullptr);
		}
	}

	for (const FVulkanPipeline& Pipeline : Pipelines)
	{
		vkDestroyPipelineLayout(Device, Pipeline.Layout, nullptr);
		vkDestroyPipeline(Device, Pipeline.Pipeline, nullptr);

		Context->DestroyObject(Pipeline.VertexShader);
		Context->DestroyObject(Pipeline.FragmentShader);
	}
}

void FVulkanMeshRenderer::CreateDescriptorSetLayout()
{
	VkDevice Device = Context->GetDevice();

	VkDescriptorSetLayoutBinding UBOBinding{};
	UBOBinding.binding = 0;
	UBOBinding.descriptorCount = 1;
	UBOBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	UBOBinding.pImmutableSamplers = nullptr;
	UBOBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding SamplerBinding{};
	SamplerBinding.binding = 1;
	SamplerBinding.descriptorCount = 1;
	SamplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	SamplerBinding.pImmutableSamplers = nullptr;
	SamplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::vector<VkDescriptorSetLayoutBinding> Bindings =
	{
		UBOBinding,
		SamplerBinding,
	};

	VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCI{};
	DescriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	DescriptorSetLayoutCI.bindingCount = static_cast<uint32_t>(Bindings.size());
	DescriptorSetLayoutCI.pBindings = Bindings.data();

	if (vkCreateDescriptorSetLayout(Device, &DescriptorSetLayoutCI, nullptr, &DescriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor set layout.");
	}
}

void FVulkanMeshRenderer::GatherInstancedDrawingInfo()
{
	FVulkanScene* Scene = GEngine->GetScene();
	if (Scene == nullptr)
	{
		return;
	}

	InstancedDrawingMap.clear();
	for (FVulkanModel* Model : Scene->GetModels())
	{
		if (Model == nullptr)
		{
			continue;
		}

		FVulkanMesh* Mesh = Model->GetMesh();
		if (Mesh == nullptr)
		{
			continue;
		}

		auto Iter = InstancedDrawingMap.find(Mesh);
		if (Iter == InstancedDrawingMap.end())
		{
			Iter = InstancedDrawingMap.insert({ Mesh, {} }).first;
		}

		Iter->second.Models.push_back(Model);
	}
}

void FVulkanMeshRenderer::CreateGraphicsPipelines()
{
	VkDevice Device = Context->GetDevice();

	std::string ShaderDirectory;
	GConfig->Get("ShaderDirectory", ShaderDirectory);

	FVulkanPipeline VertPhongPipeline;
	VertPhongPipeline.VertexShader = Context->CreateObject<FVulkanShader>();
	VertPhongPipeline.VertexShader->LoadFile(ShaderDirectory + "vert_phong.vert.spv");
	VertPhongPipeline.FragmentShader = Context->CreateObject<FVulkanShader>();
	VertPhongPipeline.FragmentShader->LoadFile(ShaderDirectory + "vert_phong.frag.spv");

	FVulkanPipeline FragPhongPipeline;
	FragPhongPipeline.VertexShader = Context->CreateObject<FVulkanShader>();
	FragPhongPipeline.VertexShader->LoadFile(ShaderDirectory + "frag_phong.vert.spv");
	FragPhongPipeline.FragmentShader = Context->CreateObject<FVulkanShader>();
	FragPhongPipeline.FragmentShader->LoadFile(ShaderDirectory + "frag_phong.frag.spv");

	FVulkanPipeline BlinnPhongPipeline;
	BlinnPhongPipeline.VertexShader = Context->CreateObject<FVulkanShader>();
	BlinnPhongPipeline.VertexShader->LoadFile(ShaderDirectory + "blinn_phong.vert.spv");
	BlinnPhongPipeline.FragmentShader = Context->CreateObject<FVulkanShader>();
	BlinnPhongPipeline.FragmentShader->LoadFile(ShaderDirectory + "blinn_phong.frag.spv");

	Pipelines.push_back(VertPhongPipeline);
	Pipelines.push_back(FragPhongPipeline);
	Pipelines.push_back(BlinnPhongPipeline);

	for (int32_t Idx = 0; Idx < Pipelines.size(); ++Idx)
	{
		VkPipelineShaderStageCreateInfo VertexShaderStageCI{};
		VertexShaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		VertexShaderStageCI.stage = VK_SHADER_STAGE_VERTEX_BIT;
		VertexShaderStageCI.module = Pipelines[Idx].VertexShader->GetModule();
		VertexShaderStageCI.pName = "main";

		VkPipelineShaderStageCreateInfo FragmentShaderStageCI{};
		FragmentShaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		FragmentShaderStageCI.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		FragmentShaderStageCI.module = Pipelines[Idx].FragmentShader->GetModule();
		FragmentShaderStageCI.pName = "main";

		VkPipelineShaderStageCreateInfo ShaderStageCIs[] =
		{
			VertexShaderStageCI,
			FragmentShaderStageCI
		};

		std::array<VkVertexInputBindingDescription, 2> VertexInputBindingDescs{};
		VertexInputBindingDescs[0].binding = 0;
		VertexInputBindingDescs[0].stride = sizeof(FVertex);
		VertexInputBindingDescs[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		VertexInputBindingDescs[1].binding = 1;
		VertexInputBindingDescs[1].stride = sizeof(FInstanceBuffer);
		VertexInputBindingDescs[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

		std::array<VkVertexInputAttributeDescription, 15> VertexInputAttributeDescs{};
		VertexInputAttributeDescs[0].binding = 0;
		VertexInputAttributeDescs[0].location = 0;
		VertexInputAttributeDescs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		VertexInputAttributeDescs[0].offset = offsetof(FVertex, Position);

		VertexInputAttributeDescs[1].binding = 0;
		VertexInputAttributeDescs[1].location = 1;
		VertexInputAttributeDescs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		VertexInputAttributeDescs[1].offset = offsetof(FVertex, Normal);

		VertexInputAttributeDescs[2].binding = 0;
		VertexInputAttributeDescs[2].location = 2;
		VertexInputAttributeDescs[2].format = VK_FORMAT_R32G32_SFLOAT;
		VertexInputAttributeDescs[2].offset = offsetof(FVertex, TexCoords);

		for (int Idx = 0; Idx < 4; ++Idx)
		{
			VertexInputAttributeDescs[3 + Idx].binding = 1;
			VertexInputAttributeDescs[3 + Idx].location = 3 + Idx;
			VertexInputAttributeDescs[3 + Idx].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			VertexInputAttributeDescs[3 + Idx].offset = offsetof(FInstanceBuffer, Model) + sizeof(glm::vec4) * Idx;
		}

		for (int Idx = 0; Idx < 4; ++Idx)
		{
			VertexInputAttributeDescs[7 + Idx].binding = 1;
			VertexInputAttributeDescs[7 + Idx].location = 7 + Idx;
			VertexInputAttributeDescs[7 + Idx].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			VertexInputAttributeDescs[7 + Idx].offset = offsetof(FInstanceBuffer, ModelView) + sizeof(glm::vec4) * Idx;
		}

		for (int Idx = 0; Idx < 4; ++Idx)
		{
			VertexInputAttributeDescs[11 + Idx].binding = 1;
			VertexInputAttributeDescs[11 + Idx].location = 11 + Idx;
			VertexInputAttributeDescs[11 + Idx].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			VertexInputAttributeDescs[11 + Idx].offset = offsetof(FInstanceBuffer, NormalMatrix) + sizeof(glm::vec4) * Idx;
		}

		VkPipelineVertexInputStateCreateInfo VertexInputStateCI{};
		VertexInputStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		VertexInputStateCI.vertexBindingDescriptionCount = static_cast<uint32_t>(VertexInputBindingDescs.size());
		VertexInputStateCI.pVertexBindingDescriptions = VertexInputBindingDescs.data();
		VertexInputStateCI.vertexAttributeDescriptionCount = static_cast<uint32_t>(VertexInputAttributeDescs.size());
		VertexInputStateCI.pVertexAttributeDescriptions = VertexInputAttributeDescs.data();

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
		MultisampleStateCI.flags = 0;

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

		if (vkCreatePipelineLayout(Device, &PipelineLayoutCI, nullptr, &Pipelines[Idx].Layout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create pipeline layout.");
		}

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
		PipelineCI.layout = Pipelines[Idx].Layout;
		PipelineCI.renderPass = Context->GetRenderPass();
		PipelineCI.subpass = 0;
		PipelineCI.basePipelineHandle = VK_NULL_HANDLE;

		VK_ASSERT(vkCreateGraphicsPipelines(Device, VK_NULL_HANDLE, 1, &PipelineCI, nullptr, &Pipelines[Idx].Pipeline));
	}
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

	if (vkCreateSampler(Device, &SamplerCI, nullptr, &TextureSampler) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create texture sampler!");
	}
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

	UniformBuffers.resize(MAX_CONCURRENT_FRAME);
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
}

void FVulkanMeshRenderer::CreateInstanceBuffers()
{
	VkPhysicalDevice PhysicalDevice = Context->GetPhysicalDevice();
	VkDevice Device = Context->GetDevice();

	VkDeviceSize UniformBufferSize = sizeof(FUniformBufferObject);

	FVulkanScene* Scene = GEngine->GetScene();
	if (Scene == nullptr)
	{
		return;
	}

	for (auto& Pair : InstancedDrawingMap)
	{
		FVulkanMesh* Mesh = Pair.first;
		std::vector<FVulkanBuffer>& InstanceBuffers = Pair.second.InstanceBuffers;
		std::vector<FVulkanModel*>& Models = Pair.second.Models;

		uint32_t InstanceBufferSize = static_cast<uint32_t>(sizeof(FInstanceBuffer) * Models.size());

		InstanceBuffers.resize(MAX_CONCURRENT_FRAME);
		for (int Idx = 0; Idx < MAX_CONCURRENT_FRAME; ++Idx)
		{
			FVulkanBuffer& InstanceBuffer = InstanceBuffers[Idx];

			Vk::CreateBuffer(
				PhysicalDevice,
				Device,
				InstanceBufferSize,
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				InstanceBuffer.Buffer,
				InstanceBuffer.Memory);

			vkMapMemory(Device, InstanceBuffer.Memory, 0, InstanceBufferSize, 0, &InstanceBuffer.Mapped);
		}

		UpdateInstanceBuffer(Mesh);
	}
}

void FVulkanMeshRenderer::CreateDescriptorSets()
{
	VkDevice Device = Context->GetDevice();
	VkDescriptorPool DescriptorPool = Context->GetDescriptorPool();

	for (auto& Pair : InstancedDrawingMap)
	{
		FVulkanMesh* Mesh = Pair.first;
		std::vector<FVulkanModel*>& Models = Pair.second.Models;
		std::vector<VkDescriptorSet>& DescriptorSets = Pair.second.DescriptorSets;

		std::vector<VkDescriptorSetLayout> Layouts(MAX_CONCURRENT_FRAME, DescriptorSetLayout);
		VkDescriptorSetAllocateInfo DescriptorSetAllocInfo{};
		DescriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		DescriptorSetAllocInfo.descriptorPool = DescriptorPool;
		DescriptorSetAllocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_CONCURRENT_FRAME);
		DescriptorSetAllocInfo.pSetLayouts = Layouts.data();

		DescriptorSets.resize(MAX_CONCURRENT_FRAME);
		VK_ASSERT(vkAllocateDescriptorSets(Device, &DescriptorSetAllocInfo, DescriptorSets.data()));
	}

	UpdateDescriptorSets();
}

void FVulkanMeshRenderer::SetPipelineIndex(int32_t Idx)
{
	if (Idx >= Pipelines.size())
	{
		return;
	}

	CurrentPipelineIndex = Idx;
}

void FVulkanMeshRenderer::UpdateUniformBuffer()
{
	static const glm::mat4 IdentityMatrix(1.0f);

	FUniformBufferObject UBO{};
	UBO.View = ViewMatrix;
	UBO.Projection = ProjectionMatrix;
	UBO.CameraPosition = CameraPosition;

	FVulkanScene* Scene = GEngine->GetScene();
	if (Scene != nullptr)
	{
		UBO.Light = Scene->GetLight();
		UBO.Light.Position = UBO.View * glm::vec4(UBO.Light.Position, 1.0f);
	}

	memcpy(UniformBuffers[Context->GetCurrentFrame()].Mapped, &UBO, sizeof(FUniformBufferObject));
}

void FVulkanMeshRenderer::UpdateInstanceBuffer(FVulkanMesh* InMesh)
{
	if (InMesh == nullptr)
	{
		return;
	}

	auto Iter = InstancedDrawingMap.find(InMesh);
	if (Iter == InstancedDrawingMap.end())
	{
		return;
	}

	static const glm::mat4 IdentityMatrix(1.0f);

	FVulkanBuffer& InstanceBuffer = Iter->second.InstanceBuffers[Context->GetCurrentFrame()];

	const std::vector<FVulkanModel*> Models = Iter->second.Models;
	{
		std::vector<int> ModelIndices;
		ModelIndices.resize(Models.size());
		for (int Idx = 0; Idx < Models.size(); ++Idx)
		{
			ModelIndices[Idx] = Idx;
		}

		std::for_each(std::execution::par, std::begin(ModelIndices), std::end(ModelIndices), [this, &Models, &InstanceBuffer](int Idx)
		{
			FVulkanModel* Model = Models[Idx];
			if (Model == nullptr)
			{
				return;
			}

			FInstanceBuffer* InstanceBufferData = (FInstanceBuffer*)InstanceBuffer.Mapped + Idx;
			InstanceBufferData->Model = Model->GetCachedModelMatrix();
			InstanceBufferData->ModelView = ViewMatrix * InstanceBufferData->Model;
			InstanceBufferData->NormalMatrix = glm::transpose(glm::inverse(glm::mat3(InstanceBufferData->ModelView)));
		});
	}
}

void FVulkanMeshRenderer::UpdateDescriptorSets()
{
	VkDevice Device = Context->GetDevice();

	for (const auto& Pair : InstancedDrawingMap)
	{
		FVulkanMesh* Mesh = Pair.first;
		if (Mesh == nullptr)
		{
			continue;
		}

		FVulkanTexture* Texture = Mesh->GetTexture();
		if (Texture == nullptr)
		{
			continue;
		}

		const std::vector<VkDescriptorSet>& DescriptorSets = Pair.second.DescriptorSets;

		for (int32_t Idx = 0; Idx < DescriptorSets.size(); ++Idx)
		{
			std::array<VkWriteDescriptorSet, 2> DescriptorWrites{};

			VkDescriptorBufferInfo UniformBufferInfo{};
			UniformBufferInfo.buffer = UniformBuffers[Idx].Buffer;
			UniformBufferInfo.offset = 0;
			UniformBufferInfo.range = sizeof(FUniformBufferObject);

			DescriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			DescriptorWrites[0].dstSet = DescriptorSets[Idx];
			DescriptorWrites[0].dstBinding = 0;
			DescriptorWrites[0].dstArrayElement = 0;
			DescriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			DescriptorWrites[0].descriptorCount = 1;
			DescriptorWrites[0].pBufferInfo = &UniformBufferInfo;

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

	vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines[CurrentPipelineIndex].Pipeline);

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

	UpdateUniformBuffer();

	for (const auto& Pair : InstancedDrawingMap)
	{
		FVulkanMesh* Mesh = Pair.first;
		if (Mesh == nullptr)
		{
			continue;
		}

		FMesh* MeshAsset = Mesh->GetMeshAsset();
		if (MeshAsset == nullptr)
		{
			continue;
		}

		const std::vector<FVulkanModel*>& Models = Pair.second.Models;
		const FVulkanBuffer& InstanceBuffer = Pair.second.InstanceBuffers[CurrentFrame];
		VkDescriptorSet DescriptorSet = Pair.second.DescriptorSets[CurrentFrame];

		UpdateInstanceBuffer(Mesh);

		vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines[CurrentPipelineIndex].Layout, 0, 1, &DescriptorSet, 0, nullptr);

		VkBuffer VertexBuffers[] = { Mesh->GetVertexBuffer().Buffer, InstanceBuffer.Buffer };
		VkDeviceSize Offsets[] = { 0, 0 };
		vkCmdBindVertexBuffers(CommandBuffer, 0, 2, VertexBuffers, Offsets);

		vkCmdBindIndexBuffer(CommandBuffer, Mesh->GetIndexBuffer().Buffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(CommandBuffer, static_cast<uint32_t>(MeshAsset->GetIndices().size()), static_cast<uint32_t>(Models.size()), 0, 0, 0);
	}
}
