#pragma once

#include <string>

struct VulkanSettings {
    uint32_t windowWidth, windowHeight;
    std::string computeShaderFile;
    uint32_t computeShaderGroupSizeX;
    uint32_t computeShaderGroupSizeY;
};
