#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "vulkan.h"
#include <iostream>
#include <set>
#include <fstream>
#include <utility>
#include <stb_image_write.h>

const std::vector<const char*> requiredDeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

Vulkan::Vulkan(VulkanSettings settings, Scene scene) :
        settings(std::move(settings)), scene(scene) {
    createWindow();
    createInstance();
    createSurface();
    pickPhysicalDevice();
    findQueueFamilies();
    createLogicalDevice();
    createSceneBuffer();
    createRenderPassDataBuffer();
    createSummedPixelColorImage();
    createCommandPool();
    createSwapChain();
    createDescriptorSetLayout();
    createDescriptorPool();
    createDescriptorSet();
    createPipelineLayout();
    createPipeline();
    createCommandBuffer();
    createFence();
    createSemaphore();
}

Vulkan::~Vulkan() {
    device.destroyImageView(summedPixelColorImageView);
    device.destroyImage(summedPixelColorImage);
    device.freeMemory(summedPixelColorImageMemory);

    device.destroyBuffer(renderPassDataBuffer);
    device.freeMemory(renderPassDataBufferMemory);

    device.destroyBuffer(sceneBuffer);
    device.freeMemory(sceneBufferMemory);

    device.destroySemaphore(semaphore);
    device.destroyFence(fence);
    device.destroyPipeline(pipeline);
    device.destroyPipelineLayout(pipelineLayout);
    device.destroyDescriptorSetLayout(descriptorSetLayout);
    device.destroyDescriptorPool(descriptorPool);
    device.destroyImageView(swapChainImageView);
    device.destroySwapchainKHR(swapChain);
    device.destroyCommandPool(commandPool);
    device.destroy();
    instance.destroySurfaceKHR(surface);
    instance.destroy();
    window.destroy();
    vkfw::terminate();
}

void Vulkan::update() {
    vkfw::pollEvents();
}

void Vulkan::render(const RenderPassData &renderPassData) {
    updateRenderPassDataBuffer(renderPassData);

    uint32_t swapChainImageIndex = device.acquireNextImageKHR(swapChain, UINT64_MAX, semaphore).value;

    device.resetFences(fence);

    vk::SubmitInfo submitInfo = {
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffer
    };

    computeQueue.submit(1, &submitInfo, fence);

    device.waitForFences(1, &fence, true, UINT64_MAX);
    device.resetFences(fence);

    vk::PresentInfoKHR presentInfo = {
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &semaphore,
            .swapchainCount = 1,
            .pSwapchains = &swapChain,
            .pImageIndices = &swapChainImageIndex
    };

    presentQueue.presentKHR(presentInfo);
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
    vk::ApplicationInfo applicationInfo = {
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

    vk::InstanceCreateInfo instanceCreateInfo = {
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

    bool computeFamilyFound = false;
    bool presentFamilyFound = false;

    for (uint32_t i = 0; i < queueFamilies.size(); i++) {
        bool supportsGraphics = (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics)
                                == vk::QueueFlagBits::eGraphics;
        bool supportsCompute = (queueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute)
                               == vk::QueueFlagBits::eCompute;
        bool supportsPresenting = physicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), surface);

        if (supportsCompute && !supportsGraphics && !computeFamilyFound) {
            computeQueueFamily = i;
            computeFamilyFound = true;
            continue;
        }

        if (supportsPresenting && !presentFamilyFound) {
            presentQueueFamily = i;
            presentFamilyFound = true;
        }

        if (computeFamilyFound && presentFamilyFound)
            break;
    }
}

void Vulkan::createLogicalDevice() {
    float queuePriority = 1.0f;
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos = {
            {
                    .queueFamilyIndex = computeQueueFamily,
                    .queueCount = 1,
                    .pQueuePriorities = &queuePriority
            },
            {
                    .queueFamilyIndex = presentQueueFamily,
                    .queueCount = 1,
                    .pQueuePriorities = &queuePriority
            }
    };

    vk::PhysicalDeviceFeatures deviceFeatures = {};

    vk::DeviceCreateInfo deviceCreateInfo = {
            .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
            .pQueueCreateInfos = queueCreateInfos.data(),
            .enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtensions.size()),
            .ppEnabledExtensionNames = requiredDeviceExtensions.data(),
            .pEnabledFeatures = &deviceFeatures
    };

    device = physicalDevice.createDevice(deviceCreateInfo);

    computeQueue = device.getQueue(computeQueueFamily, 0);
    presentQueue = device.getQueue(presentQueueFamily, 0);
}

void Vulkan::createCommandPool() {
    commandPool = device.createCommandPool({.queueFamilyIndex = computeQueueFamily});
}

void Vulkan::createSwapChain() {
    vk::SwapchainCreateInfoKHR swapChainCreateInfo = {
            .surface = surface,
            .minImageCount = 1,
            .imageFormat = swapChainImageFormat,
            .imageColorSpace = colorSpace,
            .imageExtent = {.width = settings.windowWidth, .height = settings.windowHeight},
            .imageArrayLayers = 1,
            .imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eStorage |
                          vk::ImageUsageFlagBits::eTransferSrc,
            .imageSharingMode = vk::SharingMode::eExclusive,
            .preTransform = physicalDevice.getSurfaceCapabilitiesKHR(surface).currentTransform,
            .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
            .presentMode = presentMode,
            .clipped = true,
            .oldSwapchain = nullptr
    };

    swapChain = device.createSwapchainKHR(swapChainCreateInfo);

    // swap chain images
    std::vector<vk::Image> swapChainImages = device.getSwapchainImagesKHR(swapChain);
    swapChainImage = swapChainImages.front();
    swapChainImageView = createImageView(swapChainImage, swapChainImageFormat);
}

vk::ImageView Vulkan::createImageView(const vk::Image &image, const vk::Format &format) const {
    return device.createImageView(
            {
                    .image = image,
                    .viewType = vk::ImageViewType::e2D,
                    .format = format,
                    .subresourceRange = {
                            .aspectMask = vk::ImageAspectFlagBits::eColor,
                            .baseMipLevel = 0,
                            .levelCount = 1,
                            .baseArrayLayer = 0,
                            .layerCount = 1
                    }
            });
}

void Vulkan::createDescriptorSetLayout() {
    std::vector<vk::DescriptorSetLayoutBinding> bindings = {
            {
                    .binding = 0,
                    .descriptorType = vk::DescriptorType::eStorageImage,
                    .descriptorCount = 1,
                    .stageFlags = vk::ShaderStageFlagBits::eCompute
            },
            {
                    .binding = 1,
                    .descriptorType = vk::DescriptorType::eStorageImage,
                    .descriptorCount = 1,
                    .stageFlags = vk::ShaderStageFlagBits::eCompute
            },
            {
                    .binding = 2,
                    .descriptorType = vk::DescriptorType::eUniformBuffer,
                    .descriptorCount = 1,
                    .stageFlags = vk::ShaderStageFlagBits::eCompute
            },
            {
                    .binding = 3,
                    .descriptorType = vk::DescriptorType::eUniformBuffer,
                    .descriptorCount = 1,
                    .stageFlags = vk::ShaderStageFlagBits::eCompute
            }
    };

    descriptorSetLayout = device.createDescriptorSetLayout(
            {
                    .bindingCount = static_cast<uint32_t>(bindings.size()),
                    .pBindings = bindings.data()
            });
}

void Vulkan::createDescriptorPool() {
    std::vector<vk::DescriptorPoolSize> poolSizes = {
            {
                    .type = vk::DescriptorType::eStorageImage,
                    .descriptorCount = 2
            },
            {
                    .type = vk::DescriptorType::eUniformBuffer,
                    .descriptorCount = 2
            }
    };

    descriptorPool = device.createDescriptorPool(
            {
                    .maxSets = 1,
                    .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
                    .pPoolSizes = poolSizes.data()
            });
}

void Vulkan::createDescriptorSet() {
    descriptorSet = device.allocateDescriptorSets(
            {
                    .descriptorPool = descriptorPool,
                    .descriptorSetCount = 1,
                    .pSetLayouts = &descriptorSetLayout
            }).front();


    vk::DescriptorImageInfo renderTargetImageInfo = {
            .imageView = swapChainImageView,
            .imageLayout = vk::ImageLayout::eGeneral
    };

    vk::DescriptorImageInfo summedPixelColorImageInfo = {
            .imageView = summedPixelColorImageView,
            .imageLayout = vk::ImageLayout::eGeneral
    };

    vk::DescriptorBufferInfo sceneBufferInfo = {
            .buffer = sceneBuffer,
            .offset = 0,
            .range = sizeof(Scene)
    };

    vk::DescriptorBufferInfo renderPassDataBufferInfo = {
            .buffer = renderPassDataBuffer,
            .offset = 0,
            .range = sizeof(RenderPassData)
    };

    std::vector<vk::WriteDescriptorSet> descriptorWrites = {
            {
                    .dstSet = descriptorSet,
                    .dstBinding = 0,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = vk::DescriptorType::eStorageImage,
                    .pImageInfo = &renderTargetImageInfo
            },
            {
                    .dstSet = descriptorSet,
                    .dstBinding = 1,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = vk::DescriptorType::eStorageImage,
                    .pImageInfo = &summedPixelColorImageInfo
            },
            {
                    .dstSet = descriptorSet,
                    .dstBinding = 2,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = vk::DescriptorType::eUniformBuffer,
                    .pBufferInfo = &sceneBufferInfo
            },
            {
                    .dstSet = descriptorSet,
                    .dstBinding = 3,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = vk::DescriptorType::eUniformBuffer,
                    .pBufferInfo = &renderPassDataBufferInfo
            }
    };

    device.updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(),
                                0, nullptr);
}

void Vulkan::createPipelineLayout() {
    pipelineLayout = device.createPipelineLayout(
            {
                    .setLayoutCount = 1,
                    .pSetLayouts = &descriptorSetLayout,
                    .pushConstantRangeCount = 0,
                    .pPushConstantRanges = nullptr
            });
}

void Vulkan::createPipeline() {
    std::vector<char> computeShaderCode = readBinaryFile(settings.computeShaderFile);

    vk::ShaderModuleCreateInfo shaderModuleCreateInfo = {
            .codeSize = computeShaderCode.size(),
            .pCode = reinterpret_cast<const uint32_t*>(computeShaderCode.data())
    };

    vk::ShaderModule computeShaderModule = device.createShaderModule(shaderModuleCreateInfo);

    vk::PipelineShaderStageCreateInfo shaderStage = {
            .stage = vk::ShaderStageFlagBits::eCompute,
            .module = computeShaderModule,
            .pName = "main",
    };

    vk::ComputePipelineCreateInfo pipelineCreateInfo = {
            .stage = shaderStage,
            .layout = pipelineLayout
    };

    pipeline = device.createComputePipeline(nullptr, pipelineCreateInfo).value;

    device.destroyShaderModule(computeShaderModule);
}

std::vector<char> Vulkan::readBinaryFile(const std::string &path) {
    std::ifstream file(path, std::ios::ate | std::ios::binary);

    if (!file.is_open())
        throw std::runtime_error("[Error] Failed to open file at '" + path + "'!");

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

void Vulkan::createCommandBuffer() {
    commandBuffer = device.allocateCommandBuffers(
            {
                    .commandPool = commandPool,
                    .level = vk::CommandBufferLevel::ePrimary,
                    .commandBufferCount = 1
            }).front();

    vk::CommandBufferBeginInfo beginInfo = {};
    commandBuffer.begin(&beginInfo);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline);

    std::vector<vk::DescriptorSet> descriptorSets = {descriptorSet};
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipelineLayout, 0, descriptorSets, nullptr);


    vk::ImageMemoryBarrier imageBarriersToGeneral[2] = {
            getImagePipelineBarrier(
                    vk::AccessFlagBits::eNoneKHR, vk::AccessFlagBits::eShaderWrite,
                    vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral, swapChainImage),
            getImagePipelineBarrier(
                    vk::AccessFlagBits::eNoneKHR, vk::AccessFlagBits::eShaderWrite,
                    vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral, summedPixelColorImage)
    };

    commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader,
                                  vk::DependencyFlagBits::eByRegion, 0, nullptr,
                                  0, nullptr, 2, imageBarriersToGeneral);

    commandBuffer.dispatch(
            static_cast<uint32_t>(std::ceil(float(settings.windowWidth) / float(settings.computeShaderGroupSize))),
            static_cast<uint32_t>(std::ceil(float(settings.windowHeight) / float(settings.computeShaderGroupSize))),
            1);

    vk::ImageMemoryBarrier imageBarrierToPresent = getImagePipelineBarrier(
            vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eMemoryRead,
            vk::ImageLayout::eGeneral, vk::ImageLayout::ePresentSrcKHR, swapChainImage);
    commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eBottomOfPipe,
                                  vk::DependencyFlagBits::eByRegion, 0, nullptr,
                                  0, nullptr, 1, &imageBarrierToPresent);

    commandBuffer.end();
}

void Vulkan::createFence() {
    fence = device.createFence({});
}

void Vulkan::createSemaphore() {
    semaphore = device.createSemaphore({});
}

uint32_t Vulkan::findMemoryTypeIndex(const uint32_t &memoryTypeBits, const vk::MemoryPropertyFlags &properties) {
    vk::PhysicalDeviceMemoryProperties memoryProperties = physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if ((memoryTypeBits & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("Unable to find suitable memory type!");
}

void Vulkan::saveScreenshot(const std::string &name) {
    vk::Buffer screenshotBuffer = device.createBuffer(
            {
                    .size = settings.windowWidth * settings.windowHeight * 4,
                    .usage = vk::BufferUsageFlagBits::eTransferDst,
                    .sharingMode = vk::SharingMode::eExclusive
            });

    vk::MemoryRequirements memoryRequirements = device.getBufferMemoryRequirements(screenshotBuffer);

    vk::DeviceMemory screenshotBufferMemory = device.allocateMemory(
            {
                    .allocationSize = memoryRequirements.size,
                    .memoryTypeIndex = findMemoryTypeIndex(memoryRequirements.memoryTypeBits,
                                                           vk::MemoryPropertyFlagBits::eHostVisible |
                                                           vk::MemoryPropertyFlagBits::eHostCoherent)
            });

    device.bindBufferMemory(screenshotBuffer, screenshotBufferMemory, 0);

    vk::CommandBuffer screenshotCommandBuffer = device.allocateCommandBuffers(
            {
                    .commandPool = commandPool,
                    .level = vk::CommandBufferLevel::ePrimary,
                    .commandBufferCount = 1
            }).front();


    std::vector<vk::BufferImageCopy> screenshotImageCopy = {
            {
                    .bufferOffset = 0,
                    .bufferRowLength = settings.windowWidth,
                    .bufferImageHeight = settings.windowHeight,
                    .imageSubresource = {
                            .aspectMask = vk::ImageAspectFlagBits::eColor,
                            .mipLevel = 0,
                            .baseArrayLayer = 0,
                            .layerCount = 1
                    },
                    .imageOffset = {.x = 0, .y = 0, .z = 0},
                    .imageExtent = {
                            .width = settings.windowWidth,
                            .height = settings.windowHeight,
                            .depth = 1
                    },
            }
    };

    vk::ImageMemoryBarrier imageBarrierToTransferSrc = getImagePipelineBarrier(
            vk::AccessFlagBits::eMemoryRead, vk::AccessFlagBits::eMemoryRead, vk::ImageLayout::ePresentSrcKHR,
            vk::ImageLayout::eTransferSrcOptimal, swapChainImage);

    vk::CommandBufferBeginInfo beginInfo = {.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
    screenshotCommandBuffer.begin(&beginInfo);


    screenshotCommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer,
                                            vk::DependencyFlagBits::eByRegion, 0, nullptr,
                                            0, nullptr, 1, &imageBarrierToTransferSrc);

    screenshotCommandBuffer.copyImageToBuffer(swapChainImage, vk::ImageLayout::eTransferSrcOptimal, screenshotBuffer,
                                              screenshotImageCopy);

    screenshotCommandBuffer.end();

    vk::Fence screenshotFence = device.createFence({});

    vk::SubmitInfo submitInfo = {
            .commandBufferCount = 1,
            .pCommandBuffers = &screenshotCommandBuffer
    };

    computeQueue.submit(1, &submitInfo, screenshotFence);

    device.waitForFences(1, &screenshotFence, true, UINT64_MAX);
    device.destroy(screenshotFence);

    void* data = device.mapMemory(screenshotBufferMemory, 0, memoryRequirements.size);
    stbi_write_png(name.c_str(), settings.windowWidth, settings.windowHeight, 4, data, settings.windowWidth * 4);
    device.unmapMemory(screenshotBufferMemory);

    device.destroyBuffer(screenshotBuffer);
    device.freeMemory(screenshotBufferMemory);
}

vk::ImageMemoryBarrier Vulkan::getImagePipelineBarrier(
        const vk::AccessFlagBits &srcAccessFlags, const vk::AccessFlagBits &dstAccessFlags,
        const vk::ImageLayout &oldLayout, const vk::ImageLayout &newLayout,
        const vk::Image &image) const {

    return {
            .srcAccessMask = srcAccessFlags,
            .dstAccessMask = dstAccessFlags,
            .oldLayout = oldLayout,
            .newLayout = newLayout,
            .srcQueueFamilyIndex = computeQueueFamily,
            .dstQueueFamilyIndex = computeQueueFamily,
            .image = image,
            .subresourceRange = {
                    .aspectMask = vk::ImageAspectFlagBits::eColor,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1
            },
    };
}

void Vulkan::createSceneBuffer() {
    sceneBuffer = device.createBuffer(
            {
                    .size = sizeof(Scene),
                    .usage = vk::BufferUsageFlagBits::eUniformBuffer,
                    .sharingMode = vk::SharingMode::eExclusive
            });

    vk::MemoryRequirements memoryRequirements = device.getBufferMemoryRequirements(sceneBuffer);

    sceneBufferMemory = device.allocateMemory(
            {
                    .allocationSize = memoryRequirements.size,
                    .memoryTypeIndex = findMemoryTypeIndex(memoryRequirements.memoryTypeBits,
                                                           vk::MemoryPropertyFlagBits::eHostVisible |
                                                           vk::MemoryPropertyFlagBits::eHostCoherent)
            });

    device.bindBufferMemory(sceneBuffer, sceneBufferMemory, 0);

    void* data = device.mapMemory(sceneBufferMemory, 0, sizeof(Scene));
    memcpy(data, &scene, sizeof(Scene));
    device.unmapMemory(sceneBufferMemory);
}

void Vulkan::createRenderPassDataBuffer() {
    renderPassDataBuffer = device.createBuffer(
            {
                    .size = sizeof(RenderPassData),
                    .usage = vk::BufferUsageFlagBits::eUniformBuffer,
                    .sharingMode = vk::SharingMode::eExclusive
            });

    vk::MemoryRequirements memoryRequirements = device.getBufferMemoryRequirements(renderPassDataBuffer);

    renderPassDataBufferMemory = device.allocateMemory(
            {
                    .allocationSize = memoryRequirements.size,
                    .memoryTypeIndex = findMemoryTypeIndex(memoryRequirements.memoryTypeBits,
                                                           vk::MemoryPropertyFlagBits::eHostVisible |
                                                           vk::MemoryPropertyFlagBits::eHostCoherent)
            });

    device.bindBufferMemory(renderPassDataBuffer, renderPassDataBufferMemory, 0);
}

void Vulkan::updateRenderPassDataBuffer(const RenderPassData &renderPassData) {
    void* data = device.mapMemory(renderPassDataBufferMemory, 0, sizeof(RenderPassData));
    memcpy(data, &renderPassData, sizeof(RenderPassData));
    device.unmapMemory(renderPassDataBufferMemory);
}

void Vulkan::createSummedPixelColorImage() {
    vk::ImageCreateInfo imageCreateInfo = {
            .imageType = vk::ImageType::e2D,
            .format = summedPixelColorImageFormat,
            .extent = {
                    .width = settings.windowWidth,
                    .height = settings.windowHeight,
                    .depth = 1
            },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = vk::SampleCountFlagBits::e1,
            .tiling = vk::ImageTiling::eOptimal,
            .usage = vk::ImageUsageFlagBits::eStorage,
            .sharingMode = vk::SharingMode::eExclusive,
            .initialLayout = vk::ImageLayout::eUndefined
    };

    summedPixelColorImage = device.createImage(imageCreateInfo);

    vk::MemoryRequirements memoryRequirements = device.getImageMemoryRequirements(summedPixelColorImage);

    summedPixelColorImageMemory = device.allocateMemory(
            {
                    .allocationSize = memoryRequirements.size,
                    .memoryTypeIndex = findMemoryTypeIndex(memoryRequirements.memoryTypeBits,
                                                           vk::MemoryPropertyFlagBits::eDeviceLocal)
            });

    device.bindImageMemory(summedPixelColorImage, summedPixelColorImageMemory, 0);

    summedPixelColorImageView = createImageView(summedPixelColorImage, summedPixelColorImageFormat);
}
