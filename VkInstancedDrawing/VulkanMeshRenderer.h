#pragma once

#include "vulkan/vulkan.h"
#include "glfw/glfw3.h"

#include "glm/glm.hpp"

#include "VulkanObject.h"
#include "VulkanContext.h"
#include "VulkanBuffer.h"
#include "VulkanMesh.h"
#include "VulkanShader.h"
#include "VulkanModel.h"

#include "Vertex.h"

#include <vector>
#include <unordered_map>

#define MAX_CONCURRENT_FRAME 2

struct FVulkanPipeline
{
	VkPipeline Pipeline;
	VkPipelineLayout Layout;
	FVulkanShader* VertexShader;
	FVulkanShader* FragmentShader;
};

class FVulkanMeshRenderer : public FVulkanObject
{
public:
	FVulkanMeshRenderer(FVulkanContext* InContext);
	virtual ~FVulkanMeshRenderer();

	void Render();

	void SetViewMatrix(const glm::mat4& InViewMatrix) { ViewMatrix = InViewMatrix; }
	void SetProjectionMatrix(const glm::mat4& InProjectionMatrix) { ProjectionMatrix = InProjectionMatrix; }
	void SetCameraPosition(const glm::vec3& InCameraPosition) { CameraPosition = InCameraPosition; }

	void SetPipelineIndex(int32_t Idx);

protected:
	void GatherInstancedDrawingInfo();

	void CreateGraphicsPipelines();
	void CreateTextureSampler();
	void CreateUniformBuffers();
	void CreateInstanceBuffers();
	void CreateDescriptorSetLayout();
	void CreateDescriptorSets();

	void UpdateUniformBuffer();
	void UpdateInstanceBuffer(FVulkanMesh* InMesh);
	void UpdateDescriptorSets();

protected:
	std::vector<FVulkanPipeline> Pipelines;
	int32_t CurrentPipelineIndex = 0;

	VkDescriptorSetLayout DescriptorSetLayout;

	struct FInstancedDrawingInfo
	{
		std::vector<FVulkanModel*> Models;
		std::vector<FVulkanBuffer> InstanceBuffers;
		std::vector<VkDescriptorSet> DescriptorSets;
	};
	std::unordered_map<FVulkanMesh*, FInstancedDrawingInfo> InstancedDrawingMap;

	std::vector<FVulkanBuffer> UniformBuffers;

	glm::mat4 ViewMatrix;
	glm::mat4 ProjectionMatrix;
	glm::vec3 CameraPosition;

	VkSampler TextureSampler;
};

