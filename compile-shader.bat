Pushd "%~dp0"

"%VULKAN_SDK%\Bin\glslc.exe" "%~dp0shaders\shader.comp" -o "cmake-build-debug\shader.comp.spv"
