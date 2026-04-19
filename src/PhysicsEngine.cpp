#include "PhysicsEngine.hpp"
#include <cmath>
#include <iostream>
#include <algorithm>
#include <random>

PhysicsEngine::PhysicsEngine() : rng(std::random_device{}()) {
    std::cout << "PhysicsEngine: spring-based nucleus with wobble\n";
    nucleonStickDistance = 0.4f;
    collisionRadius = 0.01f;
    springStrength = 40.0f;
    wobbleStrength = 0.2f;
}

glm::vec3 randomDirection(std::mt19937& rng) {
    std::uniform_real_distribution<float> dis(0, 2 * M_PI);
    float theta = dis(rng);
    float phi = acos(2.0f * dis(rng) - 1.0f);
    return glm::vec3(sin(phi) * cos(theta), sin(phi) * sin(theta), cos(phi));
}

// ========== 温和防止重叠（不弹飞） ==========
void PhysicsEngine::preventOverlap(std::vector<Particle>& particles) {
    static int logCounter = 0;
    for (size_t i = 0; i < particles.size(); ++i) {
        if (!particles[i].active) continue;
        for (size_t j = i + 1; j < particles.size(); ++j) {
            if (!particles[j].active) continue;
            float dist = glm::distance(particles[i].pos, particles[j].pos);
            
            bool bothNucleons = (particles[i].type == ParticleType::PROTON || particles[i].type == ParticleType::NEUTRON) &&
                                (particles[j].type == ParticleType::PROTON || particles[j].type == ParticleType::NEUTRON);
            
            float minDist = bothNucleons ? 0.5f : collisionRadius;
            
            if (dist < minDist && dist > 0.01f) {
                if (logCounter++ % 60 == 0) {
                    std::cout << "Overlap: dist=" << dist << ", minDist=" << minDist << std::endl;
                }
                glm::vec3 dir = glm::normalize(particles[j].pos - particles[i].pos);
                float overlap = minDist - dist;
                // 非常温和的分离
                float correction = overlap * 0.1f;
                particles[i].pos -= dir * correction;
                particles[j].pos += dir * correction;
                
                // 如果两者速度都很大，也减缓速度
                if (glm::length(particles[i].vel) > 2.0f) {
                    particles[i].vel *= 0.9f;
                }
                if (glm::length(particles[j].vel) > 2.0f) {
                    particles[j].vel *= 0.9f;
                }
            }
        }
    }
}

// ========== 核子碰撞形成原子核 ==========
void PhysicsEngine::handleNucleonCollision(std::vector<Particle>& particles) {
    static int logCounter = 0;
    std::vector<bool> visited(particles.size(), false);
    
    for (size_t i = 0; i < particles.size(); ++i) {
        if (visited[i] || !particles[i].active) continue;
        bool isN = (particles[i].type == ParticleType::PROTON || particles[i].type == ParticleType::NEUTRON);
        if (!isN) continue;
        
        std::vector<int> group;
        group.push_back(i);
        
        for (size_t j = 0; j < particles.size(); ++j) {
            if (i == j || visited[j]) continue;
            if (!particles[j].active) continue;
            bool isN2 = (particles[j].type == ParticleType::PROTON || particles[j].type == ParticleType::NEUTRON);
            if (!isN2) continue;
            
            float dist = glm::distance(particles[i].pos, particles[j].pos);
            if (dist < nucleonStickDistance) {
                group.push_back(j);
                visited[j] = true;
                if (logCounter++ % 30 == 0) {
                    std::cout << "Nucleon collision: dist=" << dist << ", group size=" << group.size() << std::endl;
                }
            }
        }
        
        if (group.size() < 2) {
            visited[i] = false;
            continue;
        }
        
        // 标记所有组内粒子为已访问
        for (int idx : group) visited[idx] = true;
        
        // 创建原子核
        Nucleus nucleus;
        nucleus.id = nextId++;
        nucleus.center = glm::vec3(0);
        nucleus.protonCount = nucleus.neutronCount = 0;
        nucleus.wobbleTime = 0;
        
        for (int l = 0; l < 3; ++l) {
            nucleus.orbitNormal[l] = randomDirection(rng);
        }
        
        // 计算中心并添加核子
        for (int idx : group) {
            nucleus.center += particles[idx].pos;
            if (particles[idx].type == ParticleType::PROTON) nucleus.protonCount++;
            else nucleus.neutronCount++;
            nucleus.nucleonIndices.push_back(idx);
            particles[idx].isInNucleus = true;
            particles[idx].nucleusId = nucleus.id;
        }
        nucleus.center /= (float)group.size();
        
        // 计算偏移并停止核子运动
        for (int idx : group) {
            particles[idx].nucleusOffset = particles[idx].pos - nucleus.center;
            particles[idx].vel = glm::vec3(0);  // 完全停止核子运动
            particles[idx].acc = glm::vec3(0);
        }
        
        std::cout << "Nucleus formed: id=" << nucleus.id 
                  << ", protons=" << nucleus.protonCount 
                  << ", neutrons=" << nucleus.neutronCount 
                  << ", nucleons=" << group.size() << std::endl;
        
        nuclei[nucleus.id] = nucleus;
    }
}

// ========== 弹簧力（保持核子相对位置） ==========
void PhysicsEngine::applySpringForces(std::vector<Particle>& particles) {
    for (auto& pair : nuclei) {
        Nucleus& nucleus = pair.second;
        for (const auto& joint : nucleus.joints) {
            if (joint.particleA >= (int)particles.size()) continue;
            if (!particles[joint.particleA].active) continue;
            
            if (joint.particleB == -1) {
                // 连接到中心点
                glm::vec3 toCenter = nucleus.center - particles[joint.particleA].pos;
                float dist = glm::length(toCenter);
                if (dist > 0.01f) {
                    float springForce = joint.strength * (dist - joint.restLength);
                    // 限制最大力
                    springForce = std::min(std::abs(springForce), 5.0f) * (springForce > 0 ? 1 : -1);
                    glm::vec3 force = (toCenter / dist) * springForce;
                    particles[joint.particleA].acc += force / particles[joint.particleA].mass;
                }
            } else {
                if (joint.particleB >= (int)particles.size()) continue;
                if (!particles[joint.particleB].active) continue;
                
                glm::vec3 delta = particles[joint.particleB].pos - particles[joint.particleA].pos;
                float dist = glm::length(delta);
                if (dist > 0.01f) {
                    float springForce = joint.strength * (dist - joint.restLength);
                    springForce = std::min(std::abs(springForce), 5.0f) * (springForce > 0 ? 1 : -1);
                    glm::vec3 force = (delta / dist) * springForce;
                    particles[joint.particleA].acc += force / particles[joint.particleA].mass;
                    particles[joint.particleB].acc -= force / particles[joint.particleB].mass;
                }
            }
        }
    }
}

// ========== 抖动 ==========
void PhysicsEngine::applyWobble(std::vector<Particle>& particles, float dt) {
    for (auto& pair : nuclei) {
        Nucleus& nucleus = pair.second;
        nucleus.wobbleTime += dt;
        float wobble = wobbleStrength * sin(nucleus.wobbleTime * 15.0f);
        
        for (int idx : nucleus.nucleonIndices) {
            if (idx >= (int)particles.size()) continue;
            std::uniform_real_distribution<float> dis(-0.01f, 0.01f);
            particles[idx].vel += glm::vec3(dis(rng), dis(rng), dis(rng)) * wobble;
        }
    }
}

// ========== 电子捕获 ==========
void PhysicsEngine::handleElectronCapture(std::vector<Particle>& particles) {
    for (auto& pair : nuclei) {
        Nucleus& nucleus = pair.second;
        if (nucleus.protonCount == 0) continue;
        
        for (size_t i = 0; i < particles.size(); ++i) {
            if (!particles[i].active) continue;
            if (particles[i].type != ParticleType::ELECTRON) continue;
            if (particles[i].inOrbit) continue;
            
            float dist = glm::distance(particles[i].pos, nucleus.center);
            int layer = -1;
            for (int l = 0; l < 3; ++l) {
                if (dist < electronCaptureDistances[l]) {
                    layer = l;
                    break;
                }
            }
            if (layer == -1) continue;
            
            int currentCount = 0;
            for (int eid : nucleus.electronIndices) {
                if (eid >= 0 && eid < (int)particles.size() && particles[eid].inOrbit && particles[eid].orbitLayer == layer) {
                    currentCount++;
                }
            }
            if (currentCount >= nucleus.maxElectronsPerLayer[layer]) continue;
            
            particles[i].inOrbit = true;
            particles[i].nucleusId = nucleus.id;
            particles[i].orbitLayer = layer;
            particles[i].orbitIndex = currentCount;
            nucleus.electronIndices.push_back(i);
            
            int total = nucleus.maxElectronsPerLayer[layer];
            float angleStep = 2.0f * M_PI / total;
            float angle = particles[i].orbitIndex * angleStep;
            
            if (nucleus.electronAngles.size() <= (size_t)i) {
                nucleus.electronAngles.resize(particles.size(), 0);
            }
            nucleus.electronAngles[i] = angle;
            
            std::cout << "Electron captured: layer=" << layer << " (" << (currentCount+1) << "/" << total << ")" << std::endl;
        }
    }
}

// ========== 更新轨道 ==========
void PhysicsEngine::updateOrbits(std::vector<Particle>& particles, float dt) {
    for (auto& pair : nuclei) {
        Nucleus& nucleus = pair.second;
        for (int eid : nucleus.electronIndices) {
            if (eid >= (int)particles.size() || !particles[eid].active) continue;
            int layer = particles[eid].orbitLayer;
            if (layer < 0 || layer >= 3) continue;
            
            float& angle = nucleus.electronAngles[eid];
            angle += orbitSpeed * dt;
            if (angle > 2 * M_PI) angle -= 2 * M_PI;
            
            float radius = electronCaptureDistances[layer];
            glm::vec3 normal = glm::normalize(nucleus.orbitNormal[layer]);
            
            glm::vec3 up = glm::vec3(0, 1, 0);
            if (std::abs(glm::dot(normal, up)) > 0.99f) up = glm::vec3(1, 0, 0);
            glm::vec3 right = glm::normalize(glm::cross(up, normal));
            glm::vec3 forward = glm::normalize(glm::cross(normal, right));
            
            glm::vec3 offset = (right * cosf(angle) + forward * sinf(angle)) * radius;
            particles[eid].pos = nucleus.center + offset;
        }
    }
}

// ========== 更新原子核中心 ==========
void PhysicsEngine::updateNucleusCenters(std::vector<Particle>& particles) {
    for (auto& pair : nuclei) {
        Nucleus& nucleus = pair.second;
        glm::vec3 newCenter(0);
        int cnt = 0;
        for (int idx : nucleus.nucleonIndices) {
            if (idx < (int)particles.size() && particles[idx].active) {
                newCenter += particles[idx].pos;
                cnt++;
            }
        }
        if (cnt > 0) {
            nucleus.center = newCenter / (float)cnt;
        }
    }
}

// ========== 主更新 ==========
void PhysicsEngine::update(std::vector<Particle>& particles, float dt) {
    static int frame = 0;
    frame++;
    
    // 防止重叠
    preventOverlap(particles);
    
    // 核子碰撞形成原子核
    handleNucleonCollision(particles);
    
    // 重置加速度
    for (auto& p : particles) {
        p.acc = glm::vec3(0);
    }
    
    // 应用弹簧力
    applySpringForces(particles);
    
    // 应用抖动
    applyWobble(particles, dt);
    
    // 更新核子位置
    for (auto& p : particles) {
        if (!p.active) continue;
        if (p.isInNucleus) {
            p.vel += p.acc * dt;
            p.vel *= 0.98f;
            p.pos += p.vel * dt;
        }
    }
    
    // 电子捕获
    handleElectronCapture(particles);
    
    // 更新轨道
    updateOrbits(particles, dt);
    
    // 更新原子核中心
    updateNucleusCenters(particles);
    
    // 游离粒子随机飘移
    std::uniform_real_distribution<float> dis(-0.2f, 0.2f);
    for (auto& p : particles) {
        if (!p.active) continue;
        if (p.isInNucleus || p.inOrbit) continue;
        p.vel += glm::vec3(dis(rng), dis(rng), dis(rng)) * 0.2f;
        p.vel *= 0.98f;
        p.pos += p.vel * dt;
        
        float bound = 20.0f;
        for (int i = 0; i < 3; ++i) {
            if (std::abs(p.pos[i]) > bound) {
                p.pos[i] = (p.pos[i] > 0) ? bound : -bound;
                p.vel[i] *= -0.5f;
            }
        }
    }
    
    // 每60帧打印一次状态
    if (frame % 60 == 0) {
        int freeNucleons = 0;
        for (const auto& p : particles) {
            if (p.active && !p.isInNucleus && (p.type == ParticleType::PROTON || p.type == ParticleType::NEUTRON)) {
                freeNucleons++;
            }
        }
        std::cout << "Status: nuclei=" << nuclei.size() << ", free nucleons=" << freeNucleons 
                  << ", electrons=" << particles.size() << std::endl;
    }
}