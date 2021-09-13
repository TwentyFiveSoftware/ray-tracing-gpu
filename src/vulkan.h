#pragma once

#define VULKAN_HPP_NO_CONSTRUCTORS
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#define VKFW_NO_STRUCT_CONSTRUCTORS

#include <vulkan/vulkan.hpp>
#include <vkfw/vkfw.hpp>
#include "vulkan_settings.h"

class Vulkan {
public:
    Vulkan(VulkanSettings settings);

    ~Vulkan();

    void update();

    [[nodiscard]] bool shouldExit() const;

private:
    VulkanSettings settings;
    const vk::Format format = vk::Format::eR8G8B8A8Unorm;
    const vk::ColorSpaceKHR colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;

    vkfw::Window window;
    vk::Instance instance;
    vk::SurfaceKHR surface;
    vk::PhysicalDevice physicalDevice;
    vk::Device device;

    uint32_t graphicsQueueFamily, presentQueueFamily, computeQueueFamily;
    vk::Queue graphicsQueue, presentQueue, computeQueue;

    vk::CommandPool graphicsCommandPool, computeCommandPool;

    vk::SwapchainKHR swapChain;
    vk::Image swapChainImage;
    vk::ImageView swapChainImageView;

    vk::RenderPass renderPass;
    vk::Framebuffer framebuffer;


    void createWindow();

    void createInstance();

    void createSurface();

    void pickPhysicalDevice();

    void findQueueFamilies();

    void createLogicalDevice();

    void createCommandPools();

    void createSwapChain();

    void createRenderPass();

    void createFramebuffer();

    [[nodiscard]] vk::ImageView createImageView(const vk::Image &image) const;

};
