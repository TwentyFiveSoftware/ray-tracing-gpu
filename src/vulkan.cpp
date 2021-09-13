#include "vulkan.h"
#include <iostream>
#include <set>

const std::vector<const char*> requiredDeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

Vulkan::Vulkan(VulkanSettings settings) : settings(settings) {
    createWindow();
    createInstance();
    createSurface();
    pickPhysicalDevice();
    findQueueFamilies();
    createLogicalDevice();
    createCommandPools();
    createSwapChain();
    createRenderPass();
}

Vulkan::~Vulkan() {
    device.destroyFramebuffer(framebuffer);
    device.destroyRenderPass(renderPass);
    device.destroyImageView(swapChainImageView);
    device.destroySwapchainKHR(swapChain);
    device.destroyCommandPool(graphicsCommandPool);
    device.destroyCommandPool(computeCommandPool);
    device.destroy();
    instance.destroySurfaceKHR(surface);
    instance.destroy();
    window.destroy();
    vkfw::terminate();
}

void Vulkan::update() {
    vkfw::pollEvents();
}

bool Vulkan::shouldExit() const {
    return window.shouldClose();
}

void Vulkan::createWindow() {
    vkfw::init();

    vkfw::WindowHints hints = {
            .clientAPI = vkfw::ClientAPI::eNone
    };

    window = vkfw::createWindow(settings.windowWidth, settings.windowHeight, "GPU Ray Tracing", hints);
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugMessageFunc(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
                                                void* pUserData) {
    std::cout << "["
              << vk::to_string(static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(messageSeverity)) << " | "
              << vk::to_string(static_cast<vk::DebugUtilsMessageTypeFlagsEXT>( messageTypes )) << "]:\n"
              << "id      : " << pCallbackData->pMessageIdName << "\n"
              << "message : " << pCallbackData->pMessage << "\n"
              << std::endl;

    return false;
}

void Vulkan::createInstance() {
    vk::ApplicationInfo applicationInfo{
            .pApplicationName = "Ray Tracing",
            .applicationVersion = 1,
            .pEngineName = "Ray Tracing",
            .engineVersion = 1,
            .apiVersion = VK_API_VERSION_1_2
    };

    auto enabledExtensions = vkfw::getRequiredInstanceExtensions();
    std::vector<const char*> enabledLayers =
            {"VK_LAYER_KHRONOS_validation", "VK_LAYER_LUNARG_monitor", "VK_LAYER_KHRONOS_synchronization2"};

    vk::DebugUtilsMessengerCreateInfoEXT debugMessengerInfo = {
            .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
                               vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                               vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                               vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose,
            .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                           vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                           vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral,
            .pfnUserCallback = &debugMessageFunc,
            .pUserData = nullptr
    };

    vk::InstanceCreateInfo instanceCreateInfo{
            .pNext = &debugMessengerInfo,
            .pApplicationInfo = &applicationInfo,
            .enabledLayerCount = static_cast<uint32_t>(enabledLayers.size()),
            .ppEnabledLayerNames = enabledLayers.data(),
            .enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size()),
            .ppEnabledExtensionNames = enabledExtensions.data(),
    };

    instance = vk::createInstance(instanceCreateInfo);
}

void Vulkan::createSurface() {
    surface = vkfw::createWindowSurface(instance, window);
}

void Vulkan::pickPhysicalDevice() {
    std::vector<vk::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();

    if (physicalDevices.empty()) {
        throw std::runtime_error("No GPU with Vulkan support found!");
    }

    for (const vk::PhysicalDevice &d: physicalDevices) {
        std::vector<vk::ExtensionProperties> availableExtensions = d.enumerateDeviceExtensionProperties();
        std::set<std::string> requiredExtensions(requiredDeviceExtensions.begin(), requiredDeviceExtensions.end());

        for (const vk::ExtensionProperties &extension: availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        if (requiredExtensions.empty()) {
            physicalDevice = d;
            return;
        }
    }

    throw std::runtime_error("No GPU supporting all required features found!");
}

void Vulkan::findQueueFamilies() {
    std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();

    bool graphicsPipelineFound = false;
    bool presentPipelineFound = false;
    bool computePipelineFound = false;

    for (uint32_t i = 0; i < queueFamilies.size(); i++) {
        bool supportsGraphics = (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics)
                                == vk::QueueFlagBits::eGraphics;
        bool supportsCompute = (queueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute)
                               == vk::QueueFlagBits::eCompute;
        bool supportsPresenting = physicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), surface);

        if (supportsGraphics && !graphicsPipelineFound) {
            graphicsQueueFamily = i;
            graphicsPipelineFound = true;
        }

        if (supportsPresenting && !presentPipelineFound) {
            presentQueueFamily = i;
            presentPipelineFound = true;
        }

        if (!supportsGraphics && supportsCompute && !computePipelineFound) {
            computeQueueFamily = i;
            computePipelineFound = true;
        }

        if (graphicsPipelineFound && presentPipelineFound && computePipelineFound)
            break;
    }

    // if no compute only queue was found, try to find one, even if it has graphics capabilities
    if (!computePipelineFound) {
        for (uint32_t i = 0; i < queueFamilies.size(); i++) {
            if ((queueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute) == vk::QueueFlagBits::eCompute) {
                computeQueueFamily = i;
                break;
            }
        }
    }
}

void Vulkan::createLogicalDevice() {
    std::set<uint32_t> uniqueQueueFamilies = {graphicsQueueFamily, presentQueueFamily, computeQueueFamily};

    float queuePriority = 1.0f;
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    queueCreateInfos.reserve(uniqueQueueFamilies.size());

    for (uint32_t queueFamily: uniqueQueueFamilies) {
        queueCreateInfos.push_back(
                {
                        .queueFamilyIndex = queueFamily,
                        .queueCount = 1,
                        .pQueuePriorities = &queuePriority
                });
    }

    vk::PhysicalDeviceFeatures deviceFeatures = {};

    vk::DeviceCreateInfo deviceCreateInfo = {
            .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
            .pQueueCreateInfos = queueCreateInfos.data(),
            .enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtensions.size()),
            .ppEnabledExtensionNames = requiredDeviceExtensions.data(),
            .pEnabledFeatures = &deviceFeatures
    };

    device = physicalDevice.createDevice(deviceCreateInfo);

    // get queues
    graphicsQueue = device.getQueue(graphicsQueueFamily, 0);
    presentQueue = device.getQueue(presentQueueFamily, 0);
    computeQueue = device.getQueue(computeQueueFamily, 0);
}

void Vulkan::createCommandPools() {
    graphicsCommandPool = device.createCommandPool({.queueFamilyIndex = graphicsQueueFamily});
    computeCommandPool = device.createCommandPool({.queueFamilyIndex = computeQueueFamily});
}

void Vulkan::createSwapChain() {
    vk::SwapchainCreateInfoKHR swapChainCreateInfo = {
            .surface = surface,
            .minImageCount = 1,
            .imageFormat = vk::Format::eB8G8R8A8Srgb,
            .imageColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear,
            .imageExtent = {.width = settings.windowWidth, .height = settings.windowHeight},
            .imageArrayLayers = 1,
            .imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eStorage,
            .imageSharingMode = vk::SharingMode::eExclusive,
            .preTransform = physicalDevice.getSurfaceCapabilitiesKHR(surface).currentTransform,
            .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
            .presentMode = vk::PresentModeKHR::eFifoRelaxed,
            .clipped = true,
            .oldSwapchain = nullptr
    };

    if (presentQueueFamily != graphicsQueueFamily) {
        uint32_t queueFamilyIndices[2] = {graphicsQueueFamily, presentQueueFamily};

        swapChainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        swapChainCreateInfo.queueFamilyIndexCount = 2;
        swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    }

    swapChain = device.createSwapchainKHR(swapChainCreateInfo);

    // swap chain images
    std::vector<vk::Image> swapChainImages = device.getSwapchainImagesKHR(swapChain);
    swapChainImage = swapChainImages.front();
    swapChainImageView = createImageView(swapChainImage);
}

void Vulkan::createRenderPass() {
    vk::AttachmentDescription attachmentDescription = {
            .format = vk::Format::eB8G8R8A8Srgb,
            .samples = vk::SampleCountFlagBits::e1,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
            .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
            .initialLayout = vk::ImageLayout::eUndefined,
            .finalLayout = vk::ImageLayout::ePresentSrcKHR
    };

    std::vector<vk::AttachmentDescription> attachments = {attachmentDescription};

    vk::AttachmentReference attachmentReference = {
            .attachment = 0,
            .layout = vk::ImageLayout::eColorAttachmentOptimal,
    };

    std::vector<vk::AttachmentReference> attachmentReferences = {attachmentReference};

    vk::SubpassDescription subpass = {
            .colorAttachmentCount = static_cast<uint32_t>(attachmentReferences.size()),
            .pColorAttachments = attachmentReferences.data()
    };

    std::vector<vk::SubpassDescription> subpasses = {subpass};

    vk::RenderPassCreateInfo renderPassCreateInfo = {
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .subpassCount = static_cast<uint32_t>(subpasses.size()),
            .pSubpasses = subpasses.data()
    };

    renderPass = device.createRenderPass(renderPassCreateInfo);
}

void Vulkan::createFramebuffer() {
    framebuffer = device.createFramebuffer(
            {
                    .renderPass = renderPass,
                    .attachmentCount = 1,
                    .pAttachments = &swapChainImageView,
                    .width = settings.windowWidth,
                    .height = settings.windowHeight,
                    .layers = 1
            }
    );
}

vk::ImageView Vulkan::createImageView(const vk::Image &image) const {
    return device.createImageView(
            {
                    .image = image,
                    .viewType = vk::ImageViewType::e2D,
                    .format = vk::Format::eB8G8R8A8Srgb,
                    .subresourceRange = {
                            .aspectMask = vk::ImageAspectFlagBits::eColor,
                            .baseMipLevel = 0,
                            .levelCount = 1,
                            .baseArrayLayer = 0,
                            .layerCount = 1
                    }
            });
}
