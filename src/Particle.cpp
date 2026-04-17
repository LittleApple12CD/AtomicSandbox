#include "Particle.hpp"
#include <cmath>

Particle::Particle(ParticleType t, glm::vec3 position)
    : pos(position), vel(0.0f), acc(0.0f), type(t), active(true) {
    getProperties(t, mass, charge, color_charge_red, color_charge_green,
                  color_charge_blue, weak_isospin, weak_hypercharge);
    stress_energy = mass * 9e16f;  // E = mc²
    lifetime = getLifetime(t);
}

void Particle::getProperties(ParticleType t, float& mass, float& charge,
                              float& cr, float& cg, float& cb,
                              float& isospin, float& hypercharge) {
    // 默认值
    mass = charge = cr = cg = cb = isospin = hypercharge = 0.0f;
    
    switch (t) {
        // 上夸克
        case ParticleType::UP_QUARK:
            mass = 2.3e-30f;      // 2.3 MeV/c²
            charge = 2.0f/3.0f * 1.6022e-19f;
            cr = 1.0f; cg = 0.0f; cb = 0.0f;  // 红色
            isospin = 0.5f;
            hypercharge = 1.0f/3.0f;
            break;
            
        // 下夸克
        case ParticleType::DOWN_QUARK:
            mass = 4.8e-30f;      // 4.8 MeV/c²
            charge = -1.0f/3.0f * 1.6022e-19f;
            cr = 0.0f; cg = 1.0f; cb = 0.0f;  // 绿色
            isospin = -0.5f;
            hypercharge = 1.0f/3.0f;
            break;
            
        // 质子 (uud)
        case ParticleType::PROTON:
            mass = 1.6726e-27f;
            charge = 1.6022e-19f;
            cr = cg = cb = 0.0f;  // 色单态
            isospin = 0.5f;
            hypercharge = 1.0f;
            break;
            
        // 中子 (udd)
        case ParticleType::NEUTRON:
            mass = 1.6749e-27f;
            charge = 0.0f;
            cr = cg = cb = 0.0f;  // 色单态
            isospin = -0.5f;
            hypercharge = 1.0f;
            break;
            
        // 电子
        case ParticleType::ELECTRON:
            mass = 9.1094e-31f;
            charge = -1.6022e-19f;
            isospin = -0.5f;
            hypercharge = -1.0f;
            break;
            
        // 光子 - 电磁力媒介
        case ParticleType::PHOTON:
            mass = 0.0f;
            charge = 0.0f;
            break;
            
        // 胶子 - 强力媒介
        case ParticleType::GLUON:
            mass = 0.0f;
            charge = 0.0f;
            cr = cg = cb = 0.5f;  // 携带色荷
            break;
            
        // W+ 玻色子 - 弱力媒介
        case ParticleType::W_BOSON:
            mass = 1.43e-25f;     // 80.4 GeV/c²
            charge = 1.6022e-19f;
            isospin = 1.0f;
            break;
            
        // Z 玻色子 - 弱力媒介
        case ParticleType::Z_BOSON:
            mass = 1.63e-25f;     // 91.2 GeV/c²
            charge = 0.0f;
            isospin = 0.0f;
            break;
            
        // 引力子（假设）
        case ParticleType::GRAVITON:
            mass = 0.0f;
            charge = 0.0f;
            break;
            
        default:
            break;
    }
}

float Particle::getLifetime(ParticleType t) {
    switch (t) {
        case ParticleType::PROTON:
        case ParticleType::ELECTRON:
        case ParticleType::PHOTON:
        case ParticleType::GRAVITON:
            return -1.0f;  // 稳定
        case ParticleType::NEUTRON:    return 880.3f;
        case ParticleType::W_BOSON:    return 3.0e-25f;
        case ParticleType::Z_BOSON:    return 3.0e-25f;
        default: return -1.0f;
    }
}

const char* Particle::typeToString(ParticleType t) {
    switch (t) {
        case ParticleType::PROTON:   return "Proton";
        case ParticleType::NEUTRON:  return "Neutron";
        case ParticleType::ELECTRON: return "Electron";
        case ParticleType::UP_QUARK: return "Up Quark";
        case ParticleType::DOWN_QUARK: return "Down Quark";
        case ParticleType::PHOTON:   return "Photon";
        case ParticleType::GLUON:    return "Gluon";
        case ParticleType::W_BOSON:  return "W Boson";
        case ParticleType::Z_BOSON:  return "Z Boson";
        case ParticleType::GRAVITON: return "Graviton";
        case ParticleType::POSITRON: return "Positron";
        default: return "Unknown";
    }
}