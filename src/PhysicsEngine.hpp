#pragma once

#include <vector>
#include <unordered_map>
#include <random>
#include <glm/glm.hpp>
#include "Particle.hpp"

struct SpringJoint {
    int particleA;
    int particleB;
    float restLength;
    float strength;
};

struct Nucleus {
    int id;
    glm::vec3 center;
    std::vector<int> nucleonIndices;
    std::vector<SpringJoint> joints;
    int protonCount, neutronCount;
    std::vector<int> electronIndices;
    std::vector<float> electronAngles;
    glm::vec3 orbitNormal[3];  // 每层轨道的法线方向（随机朝向）
    float orbitRadius[3] = {2.0f, 4.0f, 6.0f};
    int maxElectronsPerLayer[3] = {1, 2, 4};
    float wobbleTime;  // 抖动计时
};

class PhysicsEngine {
public:
    PhysicsEngine();
    void update(std::vector<Particle>& particles, float dt);
    void clearNuclei() { nuclei.clear(); nextId = 0; }
    const std::unordered_map<int, Nucleus>& getNuclei() const { return nuclei; }

private:
    void handleNucleonCollision(std::vector<Particle>& particles);
    void applySpringForces(std::vector<Particle>& particles);
    void applyWobble(std::vector<Particle>& particles, float dt);
    void handleElectronCapture(std::vector<Particle>& particles);
    void updateOrbits(std::vector<Particle>& particles, float dt);
    void updateNucleusCenters(std::vector<Particle>& particles);
    void preventOverlap(std::vector<Particle>& particles);
    
    std::unordered_map<int, Nucleus> nuclei;
    int nextId = 0;
    float nucleonStickDistance = 0.8f;
    float collisionRadius = 0.5f;
    float springStrength = 50.0f;
    float wobbleStrength = 0.5f;
    float electronCaptureDistances[3] = {2.0f, 4.0f, 6.0f};
    float orbitSpeed = 2.5f;
    
    std::mt19937 rng;
};