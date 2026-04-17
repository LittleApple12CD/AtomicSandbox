#include "PhysicsEngine.hpp"
#include <cmath>
#include <iostream>
#include <random>
#include <algorithm>

PhysicsEngine::PhysicsEngine()
    : kCoulomb(8.987551787e9f)
    , G(6.67430e-11f)
    , alpha_strong(1.0f)
    , alpha_weak(0.034f)
    , hbar(1.0545718e-34f)
    , c(299792458.0f)
    , softening(1e-5f)
    , maxForce(1e6f)
    , timeAccumulator(0.0f) {}

glm::vec3 PhysicsEngine::computeElectromagneticForce(const Particle& a, const Particle& b) {
    if (a.charge == 0.0f || b.charge == 0.0f) return glm::vec3(0.0f);
    
    glm::vec3 r_vec = b.pos - a.pos;
    float r = glm::length(r_vec);
    if (r < 0.01f) return glm::vec3(0.0f);  // 避免过近
    
    // 库仑定律
    float force_magnitude = kCoulomb * a.charge * b.charge / (r * r + softening);
    
    // 缩放因子：让力在可视范围内有效
    float scale = 1e-10f;
    force_magnitude *= scale;
    
    // 限制最大力
    force_magnitude = std::min(std::abs(force_magnitude), 10.0f) * (force_magnitude > 0 ? 1 : -1);
    
    return (r_vec / r) * force_magnitude;
}

glm::vec3 PhysicsEngine::computeStrongForce(const Particle& a, const Particle& b) {
    // 质子-中子之间的核力
    bool isNucleonA = (a.type == ParticleType::PROTON || a.type == ParticleType::NEUTRON);
    bool isNucleonB = (b.type == ParticleType::PROTON || b.type == ParticleType::NEUTRON);
    
    if (!isNucleonA || !isNucleonB) return glm::vec3(0.0f);
    
    glm::vec3 r_vec = b.pos - a.pos;
    float r = glm::length(r_vec);
    if (r < 0.01f) return glm::vec3(0.0f);
    if (r > 0.3f) return glm::vec3(0.0f);  // 核力短程
    
    // 核力势：吸引-排斥势（类似 Lennard-Jones）
    float sigma = 0.08f;   // 力程
    float epsilon = 1.0f;  // 强度
    
    // Lennard-Jones 势的导数
    float r6 = pow(sigma / r, 6);
    float r12 = r6 * r6;
    float force_magnitude = 24.0f * epsilon * (2.0f * r12 / r - r6 / r);
    
    force_magnitude = std::min(std::abs(force_magnitude), 10.0f);
    if (force_magnitude > 0) force_magnitude = -force_magnitude;  // 吸引力
    
    return (r_vec / r) * force_magnitude;
}

glm::vec3 PhysicsEngine::computeWeakForce(const Particle& a, const Particle& b) {
    // 弱力极短程，在可视尺度下忽略
    return glm::vec3(0.0f);
}

glm::vec3 PhysicsEngine::computeGravity(const Particle& a, const Particle& b) {
    // 引力太弱，在可视尺度下忽略
    return glm::vec3(0.0f);
}

void PhysicsEngine::computeForces(std::vector<Particle>& particles, float dt) {
    size_t n = particles.size();
    
    // 重置加速度
    for (auto& p : particles) {
        p.acc = glm::vec3(0.0f);
    }
    
    // 计算所有粒子对之间的力
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = i + 1; j < n; ++j) {
            if (!particles[i].active || !particles[j].active) continue;
            
            // 跳过力媒介粒子
            if (particles[i].mass == 0.0f && particles[j].mass == 0.0f) continue;
            
            // 按强度顺序叠加力
            glm::vec3 force = computeStrongForce(particles[i], particles[j]);
            force += computeElectromagneticForce(particles[i], particles[j]);
            // 弱力和引力太弱，忽略
            
            if (particles[i].mass > 0.0f) {
                particles[i].acc += force / particles[i].mass;
            }
            if (particles[j].mass > 0.0f) {
                particles[j].acc -= force / particles[j].mass;
            }
        }
    }
    
    processDecays(particles, dt);
}

void PhysicsEngine::updatePositions(std::vector<Particle>& particles, float dt) {
    for (auto& p : particles) {
        if (!p.active) continue;
        
        // 力媒介粒子（光子、胶子）缓慢飘浮
        if (p.mass == 0.0f) {
            // 给一个随机方向的速度
            if (glm::length(p.vel) < 0.01f) {
                static std::random_device rd;
                static std::mt19937 gen(rd());
                static std::uniform_real_distribution<> dis(-1.0, 1.0);
                p.vel = glm::vec3(dis(gen), dis(gen), dis(gen));
                p.vel = glm::normalize(p.vel) * 1.0f;  // 缓慢飘浮
            }
            p.pos += p.vel * dt;
            
            // 边界：1米边界，反射
            float boundary = 1.0f;
            if (std::abs(p.pos.x) > boundary) {
                p.pos.x = (p.pos.x > 0) ? boundary : -boundary;
                p.vel.x *= -0.8f;
            }
            if (std::abs(p.pos.y) > boundary) {
                p.pos.y = (p.pos.y > 0) ? boundary : -boundary;
                p.vel.y *= -0.8f;
            }
            if (std::abs(p.pos.z) > boundary) {
                p.pos.z = (p.pos.z > 0) ? boundary : -boundary;
                p.vel.z *= -0.8f;
            }
            continue;
        }
        
        // 检查数值稳定性
        if (std::isnan(p.vel.x) || std::isnan(p.vel.y) || std::isnan(p.vel.z)) {
            std::cerr << "Warning: NaN detected, resetting particle" << std::endl;
            p.vel = glm::vec3(0.0f);
            p.acc = glm::vec3(0.0f);
            continue;
        }
        
        // 欧拉积分
        p.vel += p.acc * dt;
        p.pos += p.vel * dt;
        
        // 速度限制（最大 5 m/s，让运动可见）
        float maxSpeed = 5.0f;
        if (glm::length(p.vel) > maxSpeed) {
            p.vel = glm::normalize(p.vel) * maxSpeed;
        }
        
        // 边界：1米立方体，弹性碰撞
        float boundary = 1.0f;
        if (std::abs(p.pos.x) > boundary) {
            p.pos.x = (p.pos.x > 0) ? boundary : -boundary;
            p.vel.x *= -0.9f;
        }
        if (std::abs(p.pos.y) > boundary) {
            p.pos.y = (p.pos.y > 0) ? boundary : -boundary;
            p.vel.y *= -0.9f;
        }
        if (std::abs(p.pos.z) > boundary) {
            p.pos.z = (p.pos.z > 0) ? boundary : -boundary;
            p.vel.z *= -0.9f;
        }
    }
    
    // 移除衰变的粒子
    particles.erase(
        std::remove_if(particles.begin(), particles.end(),
            [](const Particle& p) { return !p.active; }),
        particles.end()
    );
}

void PhysicsEngine::processDecays(std::vector<Particle>& particles, float dt) {
    timeAccumulator += dt;
    if (timeAccumulator < 0.1f) return;
    timeAccumulator = 0.0f;
    
    for (size_t i = 0; i < particles.size(); ++i) {
        auto& p = particles[i];
        if (!p.active) continue;
        
        if (p.lifetime > 0.0f) {
            static std::random_device rd;
            static std::mt19937 gen(rd());
            static std::uniform_real_distribution<> dis(0.0, 1.0);
            
            float probability = 0.1f / p.lifetime;
            if (dis(gen) < probability) {
                std::cout << "Decay: " << Particle::typeToString(p.type) << std::endl;
                
                // β⁻衰变：中子 → 质子 + 电子 + 反中微子
                if (p.type == ParticleType::NEUTRON) {
                    particles.emplace_back(ParticleType::PROTON, p.pos);
                    particles.emplace_back(ParticleType::ELECTRON, p.pos);
                    particles.emplace_back(ParticleType::ANTI_ELECTRON_NEUTRINO, p.pos);
                    p.active = false;
                }
                // W 玻色子衰变
                else if (p.type == ParticleType::W_BOSON) {
                    particles.emplace_back(ParticleType::ELECTRON, p.pos);
                    particles.emplace_back(ParticleType::ELECTRON_NEUTRINO, p.pos);
                    p.active = false;
                }
            }
        }
    }
}