#include "Particle.hpp"
#include <cmath>

// 删除构造函数定义（已经在头文件中内联定义了）
// 删除 getProperties 函数（未使用）

const char* Particle::typeToString(ParticleType t) {
    switch (t) {
        case ParticleType::PROTON:   return "Proton";
        case ParticleType::NEUTRON:  return "Neutron";
        case ParticleType::ELECTRON: return "Electron";
        case ParticleType::PHOTON:   return "Photon";
        case ParticleType::GLUON:    return "Gluon";
        case ParticleType::W_BOSON:  return "W Boson";
        case ParticleType::Z_BOSON:  return "Z Boson";
        case ParticleType::UP_QUARK: return "Up Quark";
        case ParticleType::DOWN_QUARK: return "Down Quark";
        case ParticleType::ELECTRON_NEUTRINO: return "Neutrino";
        case ParticleType::POSITRON: return "Positron";
        default: return "Unknown";
    }
}

float Particle::getMass(ParticleType t) {
    switch (t) {
        case ParticleType::PROTON:   return 1.6726e-27f;
        case ParticleType::NEUTRON:  return 1.6749e-27f;
        case ParticleType::ELECTRON: return 9.1094e-31f;
        case ParticleType::W_BOSON:  return 1.43e-25f;
        case ParticleType::Z_BOSON:  return 1.63e-25f;
        default: return 1.0e-27f;
    }
}

float Particle::getCharge(ParticleType t) {
    switch (t) {
        case ParticleType::PROTON:   return 1.6022e-19f;
        case ParticleType::ELECTRON: return -1.6022e-19f;
        case ParticleType::UP_QUARK: return 2.0f/3.0f * 1.6022e-19f;
        case ParticleType::DOWN_QUARK: return -1.0f/3.0f * 1.6022e-19f;
        default: return 0.0f;
    }
}

float Particle::getLifetime(ParticleType t) {
    switch (t) {
        case ParticleType::NEUTRON:    return 880.3f;
        case ParticleType::W_BOSON:    return 3.0e-25f;
        case ParticleType::Z_BOSON:    return 3.0e-25f;
        default: return -1.0f;
    }
}