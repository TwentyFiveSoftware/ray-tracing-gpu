#pragma once

#include <glm/glm.hpp>

enum MaterialType {
    DIFFUSE = 0,
    METAL = 1,
    REFRACTIVE = 2
};

struct Sphere {
    alignas(16) glm::vec3 center;
    alignas(4) float radius;
    alignas(4) uint32_t materialIndex;
};

struct Material {
    alignas(4) uint32_t type;
    alignas(16) glm::vec3 attenuation;
    alignas(4) float specificAttribute;
};

struct Scene {
    alignas(4) uint32_t sphereAmount;
    Sphere spheres[16];
    Material materials[16];
};


Scene generateRandomScene();