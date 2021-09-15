#include <chrono>
#include <iostream>
#include <thread>
#include "vulkan.h"

const uint32_t renderPasses = 100;
const uint32_t samples = 5000;

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

    auto renderBeginTime = std::chrono::steady_clock::now();

    for (uint32_t pass = 1; pass <= renderPasses; pass++) {
        RenderPassData renderPassData = {
                .number = pass,
                .renderPasses = renderPasses,
                .samples = samples
        };

        std::cout << "Render pass " << pass << " / " << renderPasses << " (" << (pass * samples / renderPasses) << " / "
                  << samples << " samples)";
        auto renderPassBeginTime = std::chrono::steady_clock::now();

        vulkan.render(renderPassData);

        auto renderPassTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - renderPassBeginTime).count();
        std::cout << " - Completed in " << renderPassTime << " ms" << std::endl;

        vulkan.update();
    }

    auto renderTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - renderBeginTime).count();
    std::cout << "Rendering completed: " << samples << " samples rendered in "
              << renderTime << " ms" << std::endl << std::endl;

    std::cout << "Saving screenshot..." << std::endl;
    vulkan.saveScreenshot("render.png");
    std::cout << "Screenshot saved" << std::endl;

    while (!vulkan.shouldExit()) {
        vulkan.update();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}
