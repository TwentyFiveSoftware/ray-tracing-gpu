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

    void render();

    [[nodiscard]] bool shouldExit() const;


private:
    VulkanSettings settings;
    const vk::Format format = vk::Format::eR8G8B8A8Unorm;
    const vk::ColorSpaceKHR colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
    const vk::PresentModeKHR presentMode = vk::PresentModeKHR::eImmediate;

    vkfw::Window window;
    vk::Instance instance;
    vk::SurfaceKHR surface;
    vk::PhysicalDevice physicalDevice;
    vk::Device device;

    uint32_t queueFamily;
    vk::Queue queue;

    vk::CommandPool commandPool;

    vk::SwapchainKHR swapChain;
    vk::Image swapChainImage;
    vk::ImageView swapChainImageView;

    vk::DescriptorSetLayout descriptorSetLayout;
    vk::DescriptorPool descriptorPool;
    vk::DescriptorSet descriptorSet;

    vk::PipelineLayout pipelineLayout;
    vk::Pipeline pipeline;

    vk::CommandBuffer commandBuffer;

    vk::Fence fence;
    vk::Semaphore semaphore;

    void createWindow();

    void createInstance();

    void createSurface();

    void pickPhysicalDevice();

    void findQueueFamily();

    void createLogicalDevice();

    void createCommandPool();

    void createSwapChain();

    [[nodiscard]] vk::ImageView createImageView(const vk::Image &image) const;

    void createDescriptorSetLayout();

    void createDescriptorPool();

    void createDescriptorSet();

    void createPipelineLayout();

    void createPipeline();

    [[nodiscard]] static std::vector<char> readBinaryFile(const std::string &path);

    void createCommandBuffer();

    void createFence();

    void createSemaphore();

};
