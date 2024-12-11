#include "VulkanMeshRenderer.h"
#include "VulkanContext.h"
#include "VulkanHelpers.h"
#include "VulkanTexture.h"
#include "VulkanScene.h"
#include "VulkanLight.h"
#include "VulkanMesh.h"
#include "VulkanMaterial.h"

#include "Utils.h"
#include "Config.h"
#include "Mesh.h"

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

struct FTransformBufferObject
{
	alignas(16) glm::mat4 View;
	alignas(16) glm::mat4 Projection;
	alignas(16) glm::vec3 CameraPosition;
};

struct FLightBufferObject
{
	alignas(8) uint32_t NumPointLights;
	FVulkanPointLight PointLights[16];

	alignas(8) uint32_t NumDirectionalLights;
	FVulkanDirectionalLight DirectionalLights[16];
};

struct FMaterialBufferObject
{
	alignas(16) glm::vec4 Ambient;
	alignas(16) glm::vec4 Diffuse;
	alignas(16) glm::vec4 Specular;
};

struct FDebugBufferObject
{
	alignas(4) bool bAttenuation;
	alignas(4) bool bGammaCorrection;
	alignas(4) bool bToneMapping;
};

struct FInstanceBuffer
{
	alignas(16) glm::mat4 Model;
	alignas(16) glm::mat4 ModelView;
	alignas(16) glm::mat4 NormalMatrix;
};

FVulkanMeshRenderer::FVulkanMeshRenderer(FVulkanContext* InContext)
	: FVulkanObject(InContext)
	, TBNPipeline(nullptr)
	, DescriptorSetLayout(VK_NULL_HANDLE)
	, Sampler(nullptr)
	, bInitialized(false)
	, bEnableTBNVisualization(false)
	, bEnableAttenuation(false)
	, bEnableGammaCorrection(false)
	, bEnableToneMapping(false)
{
	CreateTextureSampler();
	CreateDescriptorSetLayout();
	CreateTBNPipeline();
}

FVulkanMeshRenderer::~FVulkanMeshRenderer()
{
	VkDevice Device = Context->GetDevice();

	vkDestroyDescriptorSetLayout(Device, DescriptorSetLayout, nullptr);
}

void FVulkanMeshRenderer::CreateDescriptorSetLayout()
{
	VkDevice Device = Context->GetDevice();

	VkDescriptorSetLayoutBinding TransformBufferBinding{};
	TransformBufferBinding.descriptorCount = 1;
	TransformBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	TransformBufferBinding.pImmutableSamplers = nullptr;
	TransformBufferBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding LightBufferBinding{};
	LightBufferBinding.descriptorCount = 1;
	LightBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	LightBufferBinding.pImmutableSamplers = nullptr;
	LightBufferBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding MaterialBufferBinding{};
	MaterialBufferBinding.descriptorCount = 1;
	MaterialBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	MaterialBufferBinding.pImmutableSamplers = nullptr;
	MaterialBufferBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding DebugBufferBinding{};
	DebugBufferBinding.descriptorCount = 1;
	DebugBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	DebugBufferBinding.pImmutableSamplers = nullptr;
	DebugBufferBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding BaseColorSamplerBinding{};
	BaseColorSamplerBinding.descriptorCount = 1;
	BaseColorSamplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	BaseColorSamplerBinding.pImmutableSamplers = nullptr;
	BaseColorSamplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding NormalSamplerBinding{};
	NormalSamplerBinding.descriptorCount = 1;
	NormalSamplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	NormalSamplerBinding.pImmutableSamplers = nullptr;
	NormalSamplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::vector<VkDescriptorSetLayoutBinding> Bindings =
	{
		TransformBufferBinding,
		LightBufferBinding,
		MaterialBufferBinding,
		DebugBufferBinding,
		BaseColorSamplerBinding,
		NormalSamplerBinding
	};

	for (int Idx = 0; Idx < Bindings.size(); ++Idx)
	{
		Bindings[Idx].binding = Idx;
	}

	VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCI{};
	DescriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	DescriptorSetLayoutCI.bindingCount = static_cast<uint32_t>(Bindings.size());
	DescriptorSetLayoutCI.pBindings = Bindings.data();

	VK_ASSERT(vkCreateDescriptorSetLayout(Device, &DescriptorSetLayoutCI, nullptr, &DescriptorSetLayout));
}

void FVulkanMeshRenderer::GenerateInstancedDrawingInfo()
{
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

	for (auto& Pair : InstancedDrawingMap)
	{
		FVulkanMesh* Mesh = Pair.first;
		FInstancedDrawingInfo& DrawingInfo = Pair.second;
		std::vector<FVulkanBuffer*>& InstanceBuffers = DrawingInfo.InstanceBuffers;
		std::vector<FVulkanModel*>& Models = DrawingInfo.Models;

		FVulkanMaterial* Material = Mesh->GetMaterial();
		if (Material == nullptr)
		{
			continue;
		}

		FVulkanPipeline* Pipeline = Context->CreateObject<FVulkanPipeline>();

		FVulkanShader* VS = Material->GetVS();
		FVulkanShader* FS = Material->GetFS();
		if (VS == nullptr || FS == nullptr)
		{
			continue;
		}

		Pipeline->SetVertexShader(VS);
		Pipeline->SetFragmentShader(FS);

		VkPipelineShaderStageCreateInfo VertexShaderStageCI{};
		VertexShaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		VertexShaderStageCI.stage = VK_SHADER_STAGE_VERTEX_BIT;
		VertexShaderStageCI.module = VS->GetModule();
		VertexShaderStageCI.pName = "main";

		VkPipelineShaderStageCreateInfo FragmentShaderStageCI{};
		FragmentShaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		FragmentShaderStageCI.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		FragmentShaderStageCI.module = FS->GetModule();
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
		VkPipelineMultisampleStateCreateInfo MultisampleStateCI = Vk::GetMultisampleStateCI();
		VkPipelineDepthStencilStateCreateInfo DepthStencilStateCI = Vk::GetDepthStencilStateCI();

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
		PipelineCI.renderPass = Context->GetRenderPass();
		PipelineCI.subpass = 0;
		PipelineCI.basePipelineHandle = VK_NULL_HANDLE;

		Pipeline->CreatePipeline(PipelineCI);

		DrawingInfo.Pipeline = Pipeline;
	}
}

void FVulkanMeshRenderer::CreateTBNPipeline()
{
	VkDevice Device = Context->GetDevice();

	std::string ShaderDirectory;
	GConfig->Get("ShaderDirectory", ShaderDirectory);

	FVulkanShader* VS = Context->CreateObject<FVulkanShader>();
	VS->LoadFile(ShaderDirectory + "visualizeTBN.vert.spv");
	FVulkanShader* GS = Context->CreateObject<FVulkanShader>();
	GS->LoadFile(ShaderDirectory + "visualizeTBN.geom.spv");
	FVulkanShader* FS = Context->CreateObject<FVulkanShader>();
	FS->LoadFile(ShaderDirectory + "visualizeTBN.frag.spv");

	TBNPipeline = Context->CreateObject<FVulkanPipeline>();
	TBNPipeline->SetVertexShader(VS);
	TBNPipeline->SetGeometryShader(GS);
	TBNPipeline->SetFragmentShader(FS);

	VkPipelineShaderStageCreateInfo VertexShaderStageCI{};
	VertexShaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	VertexShaderStageCI.stage = VK_SHADER_STAGE_VERTEX_BIT;
	VertexShaderStageCI.module = TBNPipeline->GetVertexShader()->GetModule();
	VertexShaderStageCI.pName = "main";

	VkPipelineShaderStageCreateInfo GeometryShaderStageCI{};
	GeometryShaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	GeometryShaderStageCI.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
	GeometryShaderStageCI.module = TBNPipeline->GetGeometryShader()->GetModule();
	GeometryShaderStageCI.pName = "main";

	VkPipelineShaderStageCreateInfo FragmentShaderStageCI{};
	FragmentShaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	FragmentShaderStageCI.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	FragmentShaderStageCI.module = TBNPipeline->GetFragmentShader()->GetModule();
	FragmentShaderStageCI.pName = "main";

	std::array<VkPipelineShaderStageCreateInfo, 3> ShaderStageCIs = { VertexShaderStageCI, GeometryShaderStageCI, FragmentShaderStageCI };

	std::vector<VkVertexInputBindingDescription> VertexInputBindingDescs;
	std::vector<VkVertexInputAttributeDescription> VertexInputAttributeDescs;
	GetVertexInputBindings(VertexInputBindingDescs);
	GetVertexInputAttributes(VertexInputAttributeDescs);

	VkPipelineVertexInputStateCreateInfo VertexInputStateCI = Vk::GetVertexInputStateCI(VertexInputBindingDescs, VertexInputAttributeDescs);
	VkPipelineInputAssemblyStateCreateInfo InputAssemblyStateCI = Vk::GetInputAssemblyStateCI();
	VkPipelineViewportStateCreateInfo ViewportStateCI = Vk::GetViewportStateCI();
	VkPipelineRasterizationStateCreateInfo RasterizerCI = Vk::GetRasterizationStateCI();
	VkPipelineMultisampleStateCreateInfo MultisampleStateCI = Vk::GetMultisampleStateCI();
	VkPipelineDepthStencilStateCreateInfo DepthStencilStateCI = Vk::GetDepthStencilStateCI();

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

	TBNPipeline->CreateLayout(PipelineLayoutCI);

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
	PipelineCI.layout = TBNPipeline->GetLayout();
	PipelineCI.renderPass = Context->GetRenderPass();
	PipelineCI.subpass = 0;
	PipelineCI.basePipelineHandle = VK_NULL_HANDLE;

	TBNPipeline->CreatePipeline(PipelineCI);
}

void FVulkanMeshRenderer::CreateTextureSampler()
{
	Sampler = Context->CreateObject<FVulkanSampler>();
}

void FVulkanMeshRenderer::CreateUniformBuffers()
{
	VkPhysicalDevice PhysicalDevice = Context->GetPhysicalDevice();
	VkDevice Device = Context->GetDevice();

	struct FUniformBufferCreateInfo
	{
		VkDeviceSize BufferSize;
		std::vector<FVulkanBuffer*>& TargetBuffer;
	};

	std::vector<FUniformBufferCreateInfo> UniformBufferCIs =
	{
		{ sizeof(FTransformBufferObject), TransformBuffers },
		{ sizeof(FLightBufferObject), LightBuffers },
		{ sizeof(FMaterialBufferObject), MaterialBuffers },
		{ sizeof(FDebugBufferObject), DebugBuffers }
	};

	const uint32_t MaxConcurrentFrames = Context->GetMaxConcurrentFrames();

	for (const FUniformBufferCreateInfo& CI : UniformBufferCIs)
	{
		CI.TargetBuffer.resize(MaxConcurrentFrames);
		for (size_t Idx = 0; Idx < MaxConcurrentFrames; ++Idx)
		{
			CI.TargetBuffer[Idx] = Context->CreateObject<FVulkanBuffer>();
			CI.TargetBuffer[Idx]->SetUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
			CI.TargetBuffer[Idx]->SetProperties(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			CI.TargetBuffer[Idx]->Allocate(CI.BufferSize);
			CI.TargetBuffer[Idx]->Map();
		}
	}
}

void FVulkanMeshRenderer::CreateInstanceBuffers()
{
	VkPhysicalDevice PhysicalDevice = Context->GetPhysicalDevice();
	VkDevice Device = Context->GetDevice();

	for (auto& Pair : InstancedDrawingMap)
	{
		FVulkanMesh* Mesh = Pair.first;
		std::vector<FVulkanBuffer*>& InstanceBuffers = Pair.second.InstanceBuffers;
		std::vector<FVulkanModel*>& Models = Pair.second.Models;

		uint32_t InstanceBufferSize = sizeof(FInstanceBuffer) * Models.size();

		const uint32_t MaxConcurrentFrames = Context->GetMaxConcurrentFrames();

		InstanceBuffers.resize(MaxConcurrentFrames);
		for (int Idx = 0; Idx < MaxConcurrentFrames; ++Idx)
		{
			InstanceBuffers[Idx] = Context->CreateObject<FVulkanBuffer>();
			InstanceBuffers[Idx]->SetUsage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
			InstanceBuffers[Idx]->SetProperties(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			InstanceBuffers[Idx]->Allocate(InstanceBufferSize);
			InstanceBuffers[Idx]->Map();
		}

		UpdateInstanceBuffer(Mesh);
	}
}

void FVulkanMeshRenderer::CreateDescriptorSets()
{
	VkDevice Device = Context->GetDevice();
	VkDescriptorPool DescriptorPool = Context->GetDescriptorPool();

	const uint32_t MaxConcurrentFrames = Context->GetMaxConcurrentFrames();

	for (auto& Pair : InstancedDrawingMap)
	{
		FVulkanMesh* Mesh = Pair.first;
		std::vector<FVulkanModel*>& Models = Pair.second.Models;
		std::vector<VkDescriptorSet>& DescriptorSets = Pair.second.DescriptorSets;

		std::vector<VkDescriptorSetLayout> Layouts(MaxConcurrentFrames, DescriptorSetLayout);
		VkDescriptorSetAllocateInfo DescriptorSetAllocInfo{};
		DescriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		DescriptorSetAllocInfo.descriptorPool = DescriptorPool;
		DescriptorSetAllocInfo.descriptorSetCount = static_cast<uint32_t>(MaxConcurrentFrames);
		DescriptorSetAllocInfo.pSetLayouts = Layouts.data();

		DescriptorSets.resize(MaxConcurrentFrames);
		VK_ASSERT(vkAllocateDescriptorSets(Device, &DescriptorSetAllocInfo, DescriptorSets.data()));
	}

	UpdateDescriptorSets();
}

void FVulkanMeshRenderer::GetVertexInputBindings(std::vector<VkVertexInputBindingDescription>& OutDescs)
{
	OutDescs.resize(2);

	OutDescs[0].binding = 0;
	OutDescs[0].stride = sizeof(FVertex);
	OutDescs[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	OutDescs[1].binding = 1;
	OutDescs[1].stride = sizeof(FInstanceBuffer);
	OutDescs[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
}

void FVulkanMeshRenderer::GetVertexInputAttributes(std::vector<VkVertexInputAttributeDescription>& OutDescs)
{
	OutDescs.resize(16);
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

	for (int Idx = 0; Idx < 4; ++Idx)
	{
		OutDescs[4 + Idx].binding = 1;
		OutDescs[4 + Idx].location = 4 + Idx;
		OutDescs[4 + Idx].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		OutDescs[4 + Idx].offset = offsetof(FInstanceBuffer, Model) + sizeof(glm::vec4) * Idx;
	}

	for (int Idx = 0; Idx < 4; ++Idx)
	{
		OutDescs[8 + Idx].binding = 1;
		OutDescs[8 + Idx].location = 8 + Idx;
		OutDescs[8 + Idx].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		OutDescs[8 + Idx].offset = offsetof(FInstanceBuffer, ModelView) + sizeof(glm::vec4) * Idx;
	}

	for (int Idx = 0; Idx < 4; ++Idx)
	{
		OutDescs[12 + Idx].binding = 1;
		OutDescs[12 + Idx].location = 12 + Idx;
		OutDescs[12 + Idx].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		OutDescs[12 + Idx].offset = offsetof(FInstanceBuffer, NormalMatrix) + sizeof(glm::vec4) * Idx;
	}
}

void FVulkanMeshRenderer::WaitIdle()
{
	VkDevice Device = Context->GetDevice();

	vkDeviceWaitIdle(Device);
}

void FVulkanMeshRenderer::UpdateUniformBuffer()
{
	if (Scene == nullptr)
	{
		return;
	}

	FVulkanCamera Camera = Scene->GetCamera();

	VkExtent2D SwapchainExtent = Context->GetSwapchainExtent();

	float FOVRadians = glm::radians(Camera.FOV);
	float AspectRatio = SwapchainExtent.width / (float)SwapchainExtent.height;

	static const glm::mat4 IdentityMatrix(1.0f);

	FTransformBufferObject TBO{};
	TBO.View = Camera.View;
	TBO.Projection = glm::perspective(FOVRadians, AspectRatio, Camera.Near, Camera.Far);
	TBO.CameraPosition = Camera.Position;

	FLightBufferObject LBO{};
	if (Scene != nullptr)
	{
		const std::vector<FVulkanPointLight>& PointLights = Scene->GetPointLights();
		const std::vector<FVulkanDirectionalLight>& DirectionalLights = Scene->GetDirectionalLights();

		LBO.NumPointLights = static_cast<uint32_t>(PointLights.size());
		for (int Idx = 0; Idx < LBO.NumPointLights; ++Idx)
		{
			LBO.PointLights[Idx] = PointLights[Idx];
			LBO.PointLights[Idx].Position = TBO.View * glm::vec4(LBO.PointLights[Idx].Position, 1.0f);
		}

		LBO.NumDirectionalLights = static_cast<uint32_t>(DirectionalLights.size());
		for (int Idx = 0; Idx < LBO.NumDirectionalLights; ++Idx)
		{
			LBO.DirectionalLights[Idx] = DirectionalLights[Idx];
			LBO.DirectionalLights[Idx].Direction = TBO.View * glm::vec4(LBO.DirectionalLights[Idx].Direction, 0.0f);
		}
	}

	FDebugBufferObject DBO{};
	DBO.bAttenuation = bEnableAttenuation;
	DBO.bGammaCorrection = bEnableGammaCorrection;
	DBO.bToneMapping = bEnableToneMapping;

	uint32_t CurrentFrame = Context->GetCurrentFrame();

	memcpy(TransformBuffers[CurrentFrame]->GetMappedAddress(), &TBO, sizeof(FTransformBufferObject));
	memcpy(LightBuffers[CurrentFrame]->GetMappedAddress(), &LBO, sizeof(FLightBufferObject));
	memcpy(DebugBuffers[CurrentFrame]->GetMappedAddress(), &DBO, sizeof(FDebugBufferObject));
}

void FVulkanMeshRenderer::UpdateMaterialBuffer(FVulkanMesh* InMesh)
{
	if (InMesh == nullptr)
	{
		return;
	}

	FVulkanMaterial* Material = InMesh->GetMaterial();
	if (Material == nullptr)
	{
		return;
	}

	FMaterialBufferObject MBO{};
	MBO.Ambient = glm::vec4(Material->GetAmbient().Vec3Param, 1.0);
	MBO.Diffuse = glm::vec4(Material->GetDiffuse().Vec3Param, 1.0);
	MBO.Specular = glm::vec4(Material->GetSpecular().Vec3Param, 1.0);

	uint32_t CurrentFrame = Context->GetCurrentFrame();

	memcpy(MaterialBuffers[CurrentFrame]->GetMappedAddress(), &MBO, sizeof(FMaterialBufferObject));
}

void FVulkanMeshRenderer::UpdateInstanceBuffer(FVulkanMesh* InMesh)
{
	if (Scene == nullptr)
	{
		return;
	}

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

	FVulkanCamera Camera = Scene->GetCamera();

	glm::mat4 View = Camera.View;

	FVulkanBuffer* InstanceBuffer = Iter->second.InstanceBuffers[Context->GetCurrentFrame()];

	const std::vector<FVulkanModel*> Models = Iter->second.Models;
	{
		std::vector<int> ModelIndices;
		ModelIndices.resize(Models.size());
		for (int Idx = 0; Idx < Models.size(); ++Idx)
		{
			ModelIndices[Idx] = Idx;
		}

		std::for_each(std::execution::par, std::begin(ModelIndices), std::end(ModelIndices), [&View, &Models, &InstanceBuffer](int Idx)
		{
			FVulkanModel* Model = Models[Idx];
			if (Model == nullptr)
			{
				return;
			}

			FInstanceBuffer* InstanceBufferData = (FInstanceBuffer*)InstanceBuffer->GetMappedAddress()  + Idx;
			InstanceBufferData->Model = Model->GetModelMatrix();
			InstanceBufferData->ModelView = View * InstanceBufferData->Model;
			InstanceBufferData->NormalMatrix = glm::transpose(glm::inverse(glm::mat3(InstanceBufferData->ModelView)));
		});
	}
}

void FVulkanMeshRenderer::UpdateDescriptorSets()
{
	VkDevice Device = Context->GetDevice();

	for (const auto& Pair : InstancedDrawingMap)
	{
		FVulkanMesh* Mesh = static_cast<FVulkanMesh*>(Pair.first);
		if (Mesh == nullptr)
		{
			continue;
		}

		FVulkanMaterial* Material = Mesh->GetMaterial();
		if (Material == nullptr)
		{
			continue;
		}

		UTexture* BaseColorTextureAsset = Material->GetBaseColor().TexParam;
		if (BaseColorTextureAsset == nullptr)
		{
			continue;
		}

		FVulkanTexture* BaseColorTexture = BaseColorTextureAsset->GetRenderTexture();
		if (BaseColorTexture == nullptr)
		{
			continue;
		}

		UTexture* NormalTextureAsset = Material->GetNormal().TexParam;
		if (NormalTextureAsset == nullptr)
		{
			continue;
		}

		FVulkanTexture* NormalTexture = NormalTextureAsset->GetRenderTexture();
		if (NormalTexture == nullptr)
		{
			continue;
		}

		const std::vector<VkDescriptorSet>& DescriptorSets = Pair.second.DescriptorSets;

		for (int32_t i = 0; i < DescriptorSets.size(); ++i)
		{
			VkDescriptorBufferInfo TransformBufferInfo{};
			TransformBufferInfo.buffer = TransformBuffers[i]->GetHandle();
			TransformBufferInfo.offset = 0;
			TransformBufferInfo.range = sizeof(FTransformBufferObject);

			VkWriteDescriptorSet TransformBufferDescriptor{};
			TransformBufferDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			TransformBufferDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			TransformBufferDescriptor.pBufferInfo = &TransformBufferInfo;

			VkDescriptorBufferInfo LightBufferInfo{};
			LightBufferInfo.buffer = LightBuffers[i]->GetHandle();
			LightBufferInfo.offset = 0;
			LightBufferInfo.range = sizeof(FLightBufferObject);

			VkWriteDescriptorSet LightBufferDescriptor{};
			LightBufferDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			LightBufferDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			LightBufferDescriptor.pBufferInfo = &LightBufferInfo;

			VkDescriptorBufferInfo MaterialBufferInfo{};
			MaterialBufferInfo.buffer = MaterialBuffers[i]->GetHandle();
			MaterialBufferInfo.offset = 0;
			MaterialBufferInfo.range = sizeof(FMaterialBufferObject);

			VkWriteDescriptorSet MaterialBufferDescriptor{};
			MaterialBufferDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			MaterialBufferDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			MaterialBufferDescriptor.pBufferInfo = &MaterialBufferInfo;

			VkDescriptorBufferInfo DebugBufferInfo{};
			DebugBufferInfo.buffer = DebugBuffers[i]->GetHandle();
			DebugBufferInfo.offset = 0;
			DebugBufferInfo.range = sizeof(FDebugBufferObject);

			VkWriteDescriptorSet DebugBufferDescriptor{};
			DebugBufferDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			DebugBufferDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			DebugBufferDescriptor.pBufferInfo = &DebugBufferInfo;

			VkDescriptorImageInfo BaseColorImageInfo{};
			BaseColorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			BaseColorImageInfo.imageView = BaseColorTexture->GetImage()->GetView();
			BaseColorImageInfo.sampler = Sampler->GetSampler();

			VkWriteDescriptorSet BaseColorDescriptor{};
			BaseColorDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			BaseColorDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			BaseColorDescriptor.pImageInfo = &BaseColorImageInfo;

			VkDescriptorImageInfo NormalImageInfo{};
			NormalImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			NormalImageInfo.imageView = NormalTexture->GetImage()->GetView();
			NormalImageInfo.sampler = Sampler->GetSampler();

			VkWriteDescriptorSet NormalDescriptor{};
			NormalDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			NormalDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			NormalDescriptor.pImageInfo = &NormalImageInfo;

			std::vector<VkWriteDescriptorSet> DescriptorWrites
			{
				TransformBufferDescriptor,
				LightBufferDescriptor,
				MaterialBufferDescriptor,
				DebugBufferDescriptor,
				BaseColorDescriptor,
				NormalDescriptor
			};

			for (int j = 0; j < DescriptorWrites.size(); ++j)
			{
				DescriptorWrites[j].dstSet = DescriptorSets[i];
				DescriptorWrites[j].dstArrayElement = 0;
				DescriptorWrites[j].dstBinding = j;
				DescriptorWrites[j].descriptorCount = 1;
			}

			vkUpdateDescriptorSets(Device, static_cast<uint32_t>(DescriptorWrites.size()), DescriptorWrites.data(), 0, nullptr);
		}
	}
}

void FVulkanMeshRenderer::PreRender()
{
	if (bInitialized)
	{
		return;
	}

	GenerateInstancedDrawingInfo();

	CreateGraphicsPipelines();
	CreateUniformBuffers();
	CreateInstanceBuffers();
	CreateDescriptorSets();

	bInitialized = true;
}

void FVulkanMeshRenderer::Render()
{
	uint32_t CurrentFrame = Context->GetCurrentFrame();
	uint32_t CurrentImageIndex = Context->GetCurrentImageIndex();

	const std::vector<VkCommandBuffer>& CommandBuffers = Context->GetCommandBuffers();
	VkCommandBuffer CommandBuffer = CommandBuffers[CurrentFrame];

	VkExtent2D SwapchainExtent = Context->GetSwapchainExtent();

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

	for (const auto& Pair : InstancedDrawingMap)
	{
		FVulkanMesh* Mesh = Pair.first;
		const FInstancedDrawingInfo& DrawingInfo = Pair.second;

		if (Mesh == nullptr)
		{
			continue;
		}

		UpdateMaterialBuffer(Mesh);
		Draw(Mesh, DrawingInfo, Viewport, Scissor);
	}
}

void FVulkanMeshRenderer::Draw(
	FVulkanMesh* InMesh,
	const FInstancedDrawingInfo& InDrawingInfo,
	VkViewport& InViewport,
	VkRect2D& InScissor)
{
	if (InMesh == nullptr)
	{
		return;
	}

	FVulkanPipeline* Pipeline = InDrawingInfo.Pipeline;
	if (Pipeline == nullptr)
	{
		return;
	}

	uint32_t CurrentFrame = Context->GetCurrentFrame();

	const std::vector<VkCommandBuffer>& CommandBuffers = Context->GetCommandBuffers();
	VkCommandBuffer CommandBuffer = CommandBuffers[CurrentFrame];

	vkCmdSetViewport(CommandBuffer, 0, 1, &InViewport);
	vkCmdSetScissor(CommandBuffer, 0, 1, &InScissor);

	UpdateInstanceBuffer(InMesh);

	FVulkanBuffer* InstanceBuffer = InDrawingInfo.InstanceBuffers[CurrentFrame];
	VkDescriptorSet DescriptorSet = InDrawingInfo.DescriptorSets[CurrentFrame];

	VkBuffer VertexBuffers[] = { InMesh->GetVertexBuffer()->GetHandle(), InstanceBuffer->GetHandle() };
	VkDeviceSize Offsets[] = { 0, 0 };
	vkCmdBindVertexBuffers(CommandBuffer, 0, 2, VertexBuffers, Offsets);
	vkCmdBindIndexBuffer(CommandBuffer, InMesh->GetIndexBuffer()->GetHandle(), 0, VK_INDEX_TYPE_UINT32);

	uint32_t NumIndices = static_cast<uint32_t>(InMesh->GetMeshAsset()->GetIndices().size());
	uint32_t NumModels = static_cast<uint32_t>(InDrawingInfo.Models.size());

	if (bEnableTBNVisualization)
	{
		vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, TBNPipeline->GetPipeline());
		vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, TBNPipeline->GetLayout(), 0, 1, &DescriptorSet, 0, nullptr);
		vkCmdDrawIndexed(CommandBuffer, NumIndices, NumModels, 0, 0, 0);
	}

	vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline->GetPipeline());
	vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline->GetLayout(), 0, 1, &DescriptorSet, 0, nullptr);
	vkCmdDrawIndexed(CommandBuffer, NumIndices, NumModels, 0, 0, 0);
}
