#include "scene.h"

Scene generateRandomScene() {
    Scene scene = {
            .sphereAmount = 4,
            .spheres = {},
            .materials = {},
    };

    scene.materials[0] = {MaterialType::DIFFUSE, glm::vec3(0.5f, 0.5f, 0.5f), 0.0f};
    scene.materials[1] = {MaterialType::DIFFUSE, glm::vec3(0.4f, 0.2f, 0.1f), 0.0f};
    scene.materials[2] = {MaterialType::METAL, glm::vec3(0.7f, 0.6f, 0.5f), 0.0f};
    scene.materials[3] = {MaterialType::REFRACTIVE, glm::vec3(1.0f, 1.0f, 1.0f), 1.5f};

    scene.spheres[0] = {glm::vec3(0.0f, -1000.0f, 1.0f), 1000.0f, 0};
    scene.spheres[1] = {glm::vec3(-4.0f, 1.0f, 0.0f), 1.0f, 1};
    scene.spheres[2] = {glm::vec3(4.0f, 1.0f, 0.0f), 1.0f, 2};
    scene.spheres[3] = {glm::vec3(0.0f, 1.0f, 0.0f), 1.0f, 3};

    return scene;
}
