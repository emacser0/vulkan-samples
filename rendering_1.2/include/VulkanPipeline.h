#pragma once

#include "VulkanShader.h"

#include "vulkan/vulkan.h"

class FVulkanPipeline : public FVulkanObject
{
public:
	FVulkanPipeline(class FVulkanContext* InContext);
	virtual ~FVulkanPipeline();

	VkPipeline GetPipeline() const { return Pipeline; }
	VkPipelineLayout GetLayout() const { return Layout; }

	FVulkanShader* GetVertexShader() const { return VS; }
	FVulkanShader* GetGeometryShader() const { return GS; }
	FVulkanShader* GetFragmentShader() const { return FS; }
	void SetVertexShader(FVulkanShader* InVS) { VS = InVS; }
	void SetGeometryShader(FVulkanShader* InGS) { GS = InGS; }
	void SetFragmentShader(FVulkanShader* InFS) { FS = InFS; }

	void CreateLayout(const VkPipelineLayoutCreateInfo& CI);
	void CreatePipeline(const VkGraphicsPipelineCreateInfo& CI);

private:
	VkPipeline Pipeline;
	VkPipelineLayout Layout;
	FVulkanShader* VS;
	FVulkanShader* GS;
	FVulkanShader* FS;
};

