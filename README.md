# Ray Tracing

<img src="https://github.com/TwentyFiveSoftware/ray-tracing-gpu/blob/master/sceneRender.png">

## Overview

This is my take on [Peter Shirley's Ray Tracing in One Weekend](https://github.com/RayTracing/raytracing.github.io) book
series.

This project is using a [compute shader](https://en.wikipedia.org/wiki/Compute_kernel) dispatched on the GPU, instead of
C++ code executed on the CPU, to render the scene. In order to do this, I implemented a heavily simplified version of
the C++ code in GLSL (the shader language). This shader code is then dispatched to the GPU using
the [Vulkan API](https://vulkan.org/), where it runs in parallel to render the image.

## My Ray Tracing series

This is the second part of my 3 project series. Before this project, I followed Peter Shirley' Ray Tracing series and
wrote a multi-threaded CPU ray tracer. After this GPU ray tracing implementation using a compute shader, I made a third
version using the
dedicated [Vulkan Ray Tracing](https://www.khronos.org/blog/vulkan-ray-tracing-final-specification-release) extension
instead of a compute shader, to speed up the rendering process even further. The extension allows accessing the
dedicated [Ray Accelerators](https://www.amd.com/de/technologies/rdna-2) in the new AMD RDNA 2 GPUs or the
dedicated [RT Cores](https://www.nvidia.com/en-us/design-visualization/technologies/turing-architecture/) in NVIDIA's
RTX graphics cards. The performance differences are compared below.

- [CPU Ray Tracing](https://github.com/TwentyFiveSoftware/ray-tracing)
- [GPU Ray Tracing (Compute Shader)](https://github.com/TwentyFiveSoftware/ray-tracing-gpu)
- [GPU Ray Tracing (Vulkan Ray Tracing extension)](https://github.com/TwentyFiveSoftware/ray-tracing-gpu-vulkan)

## Performance

The performance was measured on the same scene (see image above) with the same amount of objects, the same recursive
depth, the same resolution (1920 x 1080). The measured times are averaged over multiple runs.

*Reference system: AMD Ryzen 9 5900X (12 Cores / 24 Threads) | AMD Radeon RX 6800 XT*

| | [CPU Ray Tracing](https://github.com/TwentyFiveSoftware/ray-tracing) | [GPU Ray Tracing (Compute Shader)](https://github.com/TwentyFiveSoftware/ray-tracing-gpu) | [GPU Ray Tracing (Vulkan RT extension)](https://github.com/TwentyFiveSoftware/ray-tracing-gpu-vulkan) |
| --- | --- | --- | --- |
| 1 sample / pixel | ~ 3,800 ms | 21.5 ms | 1.25 ms |
| 10,000 samples / pixel | ~ 10.5 h (extrapolated) | 215 s | 12.5 s |
