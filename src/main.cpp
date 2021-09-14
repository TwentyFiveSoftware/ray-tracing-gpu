#include "vulkan.h"
#include <chrono>
#include <iostream>

int main() {
    VulkanSettings settings = {
            .windowWidth = 1200,
            .windowHeight = 675,
            .computeShaderFile = "shader.comp.spv",
            .computeShaderGroupSize = 16
    };

    Vulkan vulkan(settings);

    auto renderBeginTime = std::chrono::steady_clock::now();

    vulkan.render();

    auto renderTime = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - renderBeginTime).count();

    std::cout << "Rendered in " << (float(renderTime) / 1000.0f) << " ms" << std::endl;

    while (!vulkan.shouldExit()) {
        vulkan.update();
    }
}
