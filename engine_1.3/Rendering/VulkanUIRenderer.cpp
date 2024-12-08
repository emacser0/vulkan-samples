#include "VulkanUIRenderer.h"
#include "VulkanContext.h"
#include "VulkanHelpers.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "Utils.h"
#include "Config.h"
#include "Widget.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_vulkan.h"
#include "imgui/imgui_impl_glfw.h"

#include "glm/gtc/matrix_transform.hpp"

#include <stdexcept>
#include <cstring>
#include <array>
#include <vector>
#include <string>

FVulkanUIRenderer::FVulkanUIRenderer(FVulkanContext* InContext)
	: FVulkanObject(InContext)
{
	ImGui::CreateContext();

	ImGuiIO& IO = ImGui::GetIO();
	ImGuiStyle& Style = ImGui::GetStyle();
	Style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
	Style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
	Style.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	Style.Colors[ImGuiCol_Header] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	Style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);

	ImGui::StyleColorsClassic();

	int32_t WindowWidth, WindowHeight;
	GConfig->Get("WindowWidth", WindowWidth);
	GConfig->Get("WindowHeight", WindowHeight);

	IO.DisplaySize = ImVec2(WindowWidth, WindowHeight);
	IO.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

	IO.FontGlobalScale = 1.0f;
	Style.ScaleAllSizes(1.0f);
}

void FVulkanUIRenderer::Destroy()
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();

	ImGui::DestroyContext();
}

void FVulkanUIRenderer::Ready()
{
	ImGui_ImplGlfw_InitForVulkan(Context->GetWindow(), true);

	VkPhysicalDevice PhysicalDevice = Context->GetPhysicalDevice();
	VkDevice Device = Context->GetDevice();
	VkCommandPool CommandPool = Context->GetCommandPool();
	VkDescriptorPool DescriptorPool = Context->GetDescriptorPool();

	//this initializes imgui for Vulkan
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = Context->GetInstance();
	init_info.PhysicalDevice = Context->GetPhysicalDevice();
	init_info.Device = Context->GetDevice();
	init_info.Queue = Context->GetGfxQueue();
	init_info.DescriptorPool = DescriptorPool;
	init_info.MinImageCount = 3;
	init_info.ImageCount = 3;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.RenderPass = Context->GetRenderPass();

	ImGui_ImplVulkan_Init(&init_info);

	ImGui_ImplVulkan_CreateFontsTexture();
	ImGui_ImplVulkan_DestroyFontsTexture();
}

void FVulkanUIRenderer::Render()
{
	ImGuiIO& IO = ImGui::GetIO();

	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplVulkan_NewFrame();

	ImGui::NewFrame();

	for (const std::shared_ptr<FWidget> Widget : Widgets)
	{
		Widget->Draw();
	}

	ImGui::Render();

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), Context->GetCommandBuffers()[Context->GetCurrentFrame()]);
}

void FVulkanUIRenderer::AddWidget(const std::shared_ptr<FWidget>& InWidget)
{
	Widgets.push_back(InWidget);
}

void FVulkanUIRenderer::RemoveWidget(const std::shared_ptr<FWidget>& InWidget)
{
	std::remove_if(Widgets.begin(), Widgets.end(), [InWidget](std::shared_ptr<FWidget> Widget)
	{
	   return Widget == InWidget;
	});
}
