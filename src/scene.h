#pragma once

#include <glm/glm.hpp>

enum MaterialType {
    DIFFUSE = 0,
    METAL = 1,
    REFRACTIVE = 2
};

enum TextureType {
    SOLID = 0,
    CHECKERED = 1
};

struct Sphere {
    alignas(16) glm::vec3 center;
    alignas(4) float radius;
    alignas(4) uint32_t materialIndex;
};

struct Color {
    alignas(16) glm::vec3 color;
};

struct Material {
    alignas(4) uint32_t type;
    alignas(4) uint32_t textureType;
    alignas(16) Color colors[2];
    alignas(4) float specificAttribute;
};

struct Scene {
    alignas(4) uint32_t sphereAmount;
    Sphere spheres[500];
    Material materials[500];
};


Scene generateRandomScene();