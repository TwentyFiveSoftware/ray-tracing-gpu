#include "scene.h"
#include <random>

float randomFloat(float min, float max) {
    std::random_device rd;
    std::mt19937 engine(rd());
    std::uniform_real_distribution<float> distribution(min, max);
    return distribution(engine);
}

float randomFloat() {
    return randomFloat(0.0f, 1.0f);
}

Scene generateRandomScene() {
    Scene scene = {
            .sphereAmount = 4,
            .spheres = {},
            .materials = {},
    };

    scene.materials[0] = {MaterialType::DIFFUSE, TextureType::CHECKERED,
                          {glm::vec3(0.05f, 0.05f, 0.05f), glm::vec3(0.95f, 0.95f, 0.95f)}, 0.0f};
    scene.materials[1] = {MaterialType::DIFFUSE, TextureType::SOLID, {glm::vec3(0.4f, 0.2f, 0.1f)}, 0.0f};
    scene.materials[2] = {MaterialType::METAL, TextureType::SOLID, {glm::vec3(0.7f, 0.6f, 0.5f)}, 0.0f};
    scene.materials[3] = {MaterialType::REFRACTIVE, TextureType::SOLID, {glm::vec3(1.0f, 1.0f, 1.0f)}, 1.5f};

    scene.spheres[0] = {glm::vec3(0.0f, -1000.0f, 1.0f), 1000.0f, 0};
    scene.spheres[1] = {glm::vec3(-4.0f, 1.0f, 0.0f), 1.0f, 1};
    scene.spheres[2] = {glm::vec3(4.0f, 1.0f, 0.0f), 1.0f, 2};
    scene.spheres[3] = {glm::vec3(0.0f, 1.0f, 0.0f), 1.0f, 3};

    uint32_t sphereIndex = 4;

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            glm::vec3 sphereCenter = glm::vec3(float(a) + 0.9f * randomFloat(), 0.2f, float(b) + 0.9f * randomFloat());

            const float materialProbability = randomFloat();
            Material material = {};

            if (materialProbability < 0.8) {
                const glm::vec3 albedo = glm::vec3(randomFloat() * randomFloat(), randomFloat() * randomFloat(),
                                                   randomFloat() * randomFloat());
                material = {
                        .type = MaterialType::DIFFUSE,
                        .textureType = TextureType::SOLID,
                        .colors = {albedo},
                        .specificAttribute = 0.0f
                };

            } else if (materialProbability < 0.95) {
                const glm::vec3 albedo = glm::vec3(randomFloat(0.5f, 1.0f), randomFloat(0.5f, 1.0f),
                                                   randomFloat(0.5f, 1.0f));
                const float fuzz = randomFloat(0.0f, 0.5f);

                material = {
                        .type = MaterialType::METAL,
                        .textureType = TextureType::SOLID,
                        .colors = {albedo},
                        .specificAttribute = fuzz
                };

            } else {
                material = {
                        .type = MaterialType::REFRACTIVE,
                        .textureType = TextureType::SOLID,
                        .colors = {glm::vec3(1.0f, 1.0f, 1.0f)},
                        .specificAttribute = 1.5f
                };

            }

            scene.materials[sphereIndex] = material;
            scene.spheres[sphereIndex] = {sphereCenter, 0.2f, sphereIndex};
            sphereIndex++;
        }
    }

    scene.sphereAmount = sphereIndex;
    return scene;
}
