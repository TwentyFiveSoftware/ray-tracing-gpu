#include "vulkan.h"
#include <thread>
#include <chrono>

const uint32_t FPS = 50;

int main() {
    VulkanSettings settings = {
            .windowWidth = 1200,
            .windowHeight = 675,
            .computeShaderFile = "shader.comp.spv",
            .computeShaderGroupCount = 16
    };

    Vulkan vulkan(settings);

    while (!vulkan.shouldExit()) {
        vulkan.update();

        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / FPS));
    }
}
