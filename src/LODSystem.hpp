#pragma once

#include <vector>
#include <glm/glm.hpp>

enum class SimulationLevel {
    SUBATOMIC,
    ATOMIC,
    MACROSCOPIC
};

class LODSystem {
public:
    LODSystem(float viewDistance = 1.0f);
    
    SimulationLevel getLevelForDistance(float distance) const;
    void setViewDistance(float distance) { viewDistance = distance; }
    float getViewDistance() const { return viewDistance; }
    
    void aggregateParticles(const std::vector<glm::vec3>& positions,
                            std::vector<glm::vec3>& macroDensities,
                            float cellSize) const;
    
private:
    float viewDistance;
    float subatomicThreshold;
    float atomicThreshold;
};