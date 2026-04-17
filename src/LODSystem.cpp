#include "LODSystem.hpp"
#include <cmath>
#include <unordered_map>

LODSystem::LODSystem(float viewDist)
    : viewDistance(viewDist)
    , subatomicThreshold(10.0f)   // 10米内都是 SUBATOMIC
    , atomicThreshold(100.0f) {}

SimulationLevel LODSystem::getLevelForDistance(float distance) const {
    if (distance < subatomicThreshold) {
        return SimulationLevel::SUBATOMIC;
    } else if (distance < atomicThreshold) {
        return SimulationLevel::ATOMIC;
    } else {
        return SimulationLevel::MACROSCOPIC;
    }
}

void LODSystem::aggregateParticles(const std::vector<glm::vec3>& positions,
                                    std::vector<glm::vec3>& macroDensities,
                                    float cellSize) const {
    macroDensities.clear();
    
    if (positions.empty()) return;
    
    // 使用哈希网格进行空间聚合
    struct CellKey {
        int x, y, z;
        bool operator==(const CellKey& other) const {
            return x == other.x && y == other.y && z == other.z;
        }
    };
    
    struct CellKeyHash {
        size_t operator()(const CellKey& k) const {
            return ((k.x * 73856093) ^ (k.y * 19349663) ^ (k.z * 83492791));
        }
    };
    
    std::unordered_map<CellKey, glm::vec3, CellKeyHash> densityMap;
    
    for (const auto& pos : positions) {
        CellKey key;
        key.x = static_cast<int>(std::floor(pos.x / cellSize));
        key.y = static_cast<int>(std::floor(pos.y / cellSize));
        key.z = static_cast<int>(std::floor(pos.z / cellSize));
        
        densityMap[key] += glm::vec3(1.0f);
    }
    
    macroDensities.reserve(densityMap.size());
    for (const auto& pair : densityMap) {
        macroDensities.push_back(pair.second);
    }
}