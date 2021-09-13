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

    vkfw::Window window;
    vk::Instance instance;
    vk::SurfaceKHR surface;
    vk::PhysicalDevice physicalDevice;
    vk::Device device;

    uint32_t graphicsQueueFamily, presentQueueFamily, computeQueueFamily;
    vk::Queue graphicsQueue, presentQueue, computeQueue;


    void createWindow();

    void createInstance();

    void createSurface();

    void pickPhysicalDevice();

    void findQueueFamilies();

    void createLogicalDevice();

};
