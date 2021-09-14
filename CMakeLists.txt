cmake_minimum_required(VERSION 3.20)
project(RayTracingGPU)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")

include_directories(
        lib/glm
        lib/stb
        lib/VulkanSDK/1.2.182.0/Include
        lib/glfw-3.3.4.bin.WIN64/include
        lib/vkfw/include
)

link_directories(
        lib/glfw-3.3.4.bin.WIN64/lib-vc2019
        lib/VulkanSDK/1.2.182.0/Lib
)

add_executable(
        RayTracingGPU
        src/main.cpp
        src/vulkan.h
        src/vulkan.cpp
        src/vulkan_settings.h
        src/render_pass_data.h
        src/scene.h
        src/scene.cpp
)

target_link_libraries(RayTracingGPU glfw3.lib vulkan-1.lib)
