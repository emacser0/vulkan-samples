#pragma once

#include "VulkanShader.h"

#include "vulkan/vulkan.h"

class FVulkanPipeline : public FVulkanObject
{
public:
	FVulkanPipeline(class FVulkanContext* InContext);

	virtual void Destroy() override;

	VkPipelineLayout GetLayout() const { return Layout; }
	VkPipeline GetPipeline() const { return Pipeline; }

	FVulkanShader* GetVertexShader() const { return VS; }
	FVulkanShader* GetGeometryShader() const { return GS; }
	FVulkanShader* GetFragmentShader() const { return FS; }
	void SetVertexShader(FVulkanShader* InVS) { VS = InVS; }
	void SetGeometryShader(FVulkanShader* InGS) { GS = InGS; }
	void SetFragmentShader(FVulkanShader* InFS) { FS = InFS; }

	void CreateLayout(const VkPipelineLayoutCreateInfo& CI);
	void CreatePipeline(const VkGraphicsPipelineCreateInfo& CI);

private:
	VkPipelineLayout Layout;
	VkPipeline Pipeline;
	FVulkanShader* VS;
	FVulkanShader* GS;
	FVulkanShader* FS;
};

