#pragma once

#include <glm/glm.hpp>
#include <cstdint>

enum class ParticleType : uint8_t {
    // 费米子（夸克）
    UP_QUARK, DOWN_QUARK,
    // 费米子（轻子）
    ELECTRON, ELECTRON_NEUTRINO,
    // 重子
    PROTON, NEUTRON,
    // 玻色子（力媒介粒子）
    PHOTON, GLUON, W_BOSON, Z_BOSON,
    GRAVITON,  // 引力子（假设）
    // 反粒子
    POSITRON, ANTI_ELECTRON_NEUTRINO,
    ANTI_UP_QUARK, ANTI_DOWN_QUARK
};

struct Particle {
    glm::vec3 pos;      // 位置 (m)
    glm::vec3 vel;      // 速度 (m/s)
    glm::vec3 acc;      // 加速度 (m/s²)
    ParticleType type;
    
    // 基本属性
    float mass;         // 质量 (kg)
    float charge;       // 电荷 (C)
    
    // 强力属性（量子色动力学 QCD）
    float color_charge_red;    // 色荷 R (0-1)
    float color_charge_green;  // 色荷 G (0-1)
    float color_charge_blue;   // 色荷 B (0-1)
    
    // 弱力属性（电弱统一理论）
    float weak_isospin;        // 弱同位旋
    float weak_hypercharge;    // 弱超荷
    
    // 引力属性
    float stress_energy;       // 应力-能量张量（简化）
    
    bool active;
    float lifetime;     // 寿命 (s)，-1表示稳定
    
    Particle(ParticleType t = ParticleType::PROTON, glm::vec3 position = glm::vec3(0.0f));
    
    static const char* typeToString(ParticleType t);
    static void getProperties(ParticleType t, float& mass, float& charge,
                              float& cr, float& cg, float& cb,
                              float& isospin, float& hypercharge);
    static float getLifetime(ParticleType t);
};