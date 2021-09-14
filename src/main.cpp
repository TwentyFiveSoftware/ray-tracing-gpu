#include <chrono>
#include <iostream>
#include <thread>
#include "vulkan.h"
#include "scene.h"

int main() {
    VulkanSettings settings = {
            .windowWidth = 1200,
            .windowHeight = 675,
            .computeShaderFile = "shader.comp.spv",
            .computeShaderGroupSize = 16
    };

    Scene scene = generateRandomScene();

    Vulkan vulkan(settings, scene);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "Start rendering..." << std::endl;

    auto renderBeginTime = std::chrono::steady_clock::now();

    vulkan.render();

    auto renderTime = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - renderBeginTime).count();

    std::cout << "Rendered in " << (float(renderTime) / 1000.0f) << " ms" << std::endl;

    std::cout << "Saving screenshot..." << std::endl;
    vulkan.saveScreenshot("render.png");
    std::cout << "Screenshot saved" << std::endl;

    while (!vulkan.shouldExit()) {
        vulkan.update();
    }
}
