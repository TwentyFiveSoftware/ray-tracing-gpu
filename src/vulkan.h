#pragma once

#define VULKAN_HPP_NO_CONSTRUCTORS
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#define VKFW_NO_STRUCT_CONSTRUCTORS

#include <vulkan/vulkan.hpp>
#include <vkfw/vkfw.hpp>
#include "vulkan_settings.h"
#include "scene.h"
#include "render_call_info.h"

struct VulkanImage {
    vk::Image image;
    vk::DeviceMemory memory;
    vk::ImageView imageView;
};

struct VulkanBuffer {
    vk::Buffer buffer;
    vk::DeviceMemory memory;
};


class Vulkan {
public:
    Vulkan(VulkanSettings settings, Scene scene);

    ~Vulkan();

    void update();

    void render(const RenderCallInfo &renderCallInfo);

    [[nodiscard]] bool shouldExit() const;

    void saveScreenshot(const std::string &name);


private:
    VulkanSettings settings;
    Scene scene;

    const vk::Format swapChainImageFormat = vk::Format::eR8G8B8A8Unorm;
    const vk::Format summedPixelColorImageFormat = vk::Format::eR16G16B16A16Unorm;
    const vk::ColorSpaceKHR colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
    const vk::PresentModeKHR presentMode = vk::PresentModeKHR::eImmediate;

    const std::vector<const char*> requiredInstanceExtensions = {
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME
    };

    const std::vector<const char*> requiredDeviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };


    vkfw::Window window;
    vk::Instance instance;
    vk::SurfaceKHR surface;
    vk::PhysicalDevice physicalDevice;
    vk::Device device;

    uint32_t computeQueueFamily = 0, presentQueueFamily = 0;
    vk::Queue computeQueue, presentQueue;

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

    VulkanBuffer sceneBuffer;
    VulkanBuffer renderCallInfoBuffer;
    VulkanImage summedPixelColorImage;

    void createWindow();

    void createInstance();

    void createSurface();

    void pickPhysicalDevice();

    void findQueueFamilies();

    void createLogicalDevice();

    void createCommandPool();

    void createSwapChain();

    [[nodiscard]] vk::ImageView createImageView(const vk::Image &image, const vk::Format &format) const;

    void createDescriptorSetLayout();

    void createDescriptorPool();

    void createDescriptorSet();

    void createPipelineLayout();

    void createPipeline();

    [[nodiscard]] static std::vector<char> readBinaryFile(const std::string &path);

    void createCommandBuffer();

    void createFence();

    void createSemaphore();

    [[nodiscard]] uint32_t findMemoryTypeIndex(const uint32_t &memoryTypeBits,
                                               const vk::MemoryPropertyFlags &properties);

    [[nodiscard]] vk::ImageMemoryBarrier getImagePipelineBarrier(
            const vk::AccessFlagBits &srcAccessFlags, const vk::AccessFlagBits &dstAccessFlags,
            const vk::ImageLayout &oldLayout, const vk::ImageLayout &newLayout, const vk::Image &image) const;

    void createSceneBuffer();

    void createRenderCallInfoBuffer();

    void updateRenderCallInfoBuffer(const RenderCallInfo &renderCallInfo);

    void createSummedPixelColorImage();

    [[nodiscard]] VulkanImage createImage(const vk::Format &format,
                                          const vk::Flags<vk::ImageUsageFlagBits> &usageFlagBits);

    void destroyImage(const VulkanImage &image) const;

    [[nodiscard]] VulkanBuffer createBuffer(const vk::DeviceSize &size, const vk::Flags<vk::BufferUsageFlagBits> &usage,
                                            const vk::Flags<vk::MemoryPropertyFlagBits> &memoryProperty);

    void destroyBuffer(const VulkanBuffer &buffer) const;

};
