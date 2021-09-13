#include "vulkan.h"

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
        vulkan.render();
    }
}
