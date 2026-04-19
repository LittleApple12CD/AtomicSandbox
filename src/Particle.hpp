#pragma once

#include <glm/glm.hpp>
#include <cstdint>

enum class ParticleType : uint8_t {
    PROTON, NEUTRON, ELECTRON, PHOTON, GLUON, W_BOSON, Z_BOSON,
    UP_QUARK, DOWN_QUARK, ELECTRON_NEUTRINO, POSITRON,
    ANTI_ELECTRON_NEUTRINO, ANTI_UP_QUARK, ANTI_DOWN_QUARK, GRAVITON
};

struct Particle {
    glm::vec3 pos;
    glm::vec3 vel;
    glm::vec3 acc;
    ParticleType type;
    float mass;
    float charge;
    float color_charge;
    float weak_isospin;
    bool active;
    float lifetime;
    
    // 原子核集群相关
    bool isInNucleus;
    int nucleusId;
    glm::vec3 nucleusOffset;
    bool inOrbit;
    int orbitLayer;
    int orbitIndex;
    
    Particle(ParticleType t = ParticleType::PROTON, glm::vec3 position = glm::vec3(0.0f))
        : pos(position), vel(0.0f), acc(0.0f), type(t), mass(getMass(t)), charge(getCharge(t))
        , color_charge(0.0f), weak_isospin(0.0f), active(true), lifetime(getLifetime(t))
        , isInNucleus(false), nucleusId(-1), nucleusOffset(0.0f), inOrbit(false), orbitLayer(-1), orbitIndex(-1) {}
    
    static const char* typeToString(ParticleType t);
    static float getMass(ParticleType t);
    static float getCharge(ParticleType t);
    static float getLifetime(ParticleType t);
};