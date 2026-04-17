#pragma once

#include <vector>
#include "Particle.hpp"

class PhysicsEngine {
public:
    PhysicsEngine();
    
    void computeForces(std::vector<Particle>& particles, float dt);
    void updatePositions(std::vector<Particle>& particles, float dt);
    
private:
    // 四种基本力
    glm::vec3 computeElectromagneticForce(const Particle& a, const Particle& b);
    glm::vec3 computeStrongForce(const Particle& a, const Particle& b);
    glm::vec3 computeWeakForce(const Particle& a, const Particle& b);
    glm::vec3 computeGravity(const Particle& a, const Particle& b);
    
    // 辅助函数
    float yukawaPotential(float r, float mass, float strength);
    void processDecays(std::vector<Particle>& particles, float dt);
    
    // 物理常数
    const float kCoulomb;           // 库仑常数
    const float G;                  // 引力常数
    const float alpha_strong;       // 强相互作用耦合常数
    const float alpha_weak;         // 弱相互作用耦合常数
    const float hbar;               // 约化普朗克常数
    const float c;                  // 光速
    
    float softening;                // 软化距离
    float maxForce;                 // 力上限
    float timeAccumulator;
};