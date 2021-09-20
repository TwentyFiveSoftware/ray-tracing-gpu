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

// https://www.codespeedy.com/hsv-to-rgb-in-cpp/
glm::vec3 getRandomColor() {
    float h = std::floor(randomFloat(0.0f, 360.0f));
    float s = 0.75f, v = 0.45f;

    float C = s * v;
    float X = C * (1.0f - std::fabs(std::fmod(h / 60.0f, 2.0f) - 1.0f));
    float m = v - C;

    float r, g, b;

    if (h >= 0 && h < 60) {
        r = C, g = X, b = 0;
    } else if (h >= 60 && h < 120) {
        r = X, g = C, b = 0;
    } else if (h >= 120 && h < 180) {
        r = 0, g = C, b = X;
    } else if (h >= 180 && h < 240) {
        r = 0, g = X, b = C;
    } else if (h >= 240 && h < 300) {
        r = X, g = 0, b = C;
    } else {
        r = C, g = 0, b = X;
    }

    return {r + m, g + m, b + m};
}

Scene generateRandomScene() {
    Scene scene = {
            .sphereAmount = 4,
            .spheres = {},
            .materials = {},
    };

    scene.materials[0] = {MaterialType::DIFFUSE, TextureType::CHECKERED,
                          {glm::vec3(0.05f, 0.05f, 0.05f), glm::vec3(0.95f, 0.95f, 0.95f)}, 0.0f};
    scene.materials[1] = {MaterialType::DIFFUSE, TextureType::SOLID, {glm::vec3(0.6f, 0.3f, 0.1f)}, 0.0f};
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

            if (materialProbability < 0.7) {
                const glm::vec3 albedo = getRandomColor();
                material = {
                        .type = MaterialType::DIFFUSE,
                        .textureType = TextureType::SOLID,
                        .colors = {albedo},
                        .specificAttribute = 0.0f
                };

            } else if (materialProbability < 0.85) {
                const glm::vec3 albedo = glm::vec3(randomFloat(0.5f, 1.0f), randomFloat(0.5f, 1.0f),
                                                   randomFloat(0.5f, 1.0f));
                const float fuzz = 0.0f;

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
