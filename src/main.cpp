#include "Simulation.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    std::cout << "=== Atomic Sandbox ===" << std::endl;
    std::cout << "A truly realistic particle simulator" << std::endl;
    std::cout << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  Space   - Pause/Resume simulation" << std::endl;
    std::cout << "  Right   - Single frame step (when paused)" << std::endl;
    std::cout << "  Scroll  - Adjust time scale (1e-6 to 1000x)" << std::endl;
    std::cout << "  1/2/3   - Select brush: Proton/Neutron/Electron" << std::endl;
    std::cout << "  RMB     - Add particle at cursor" << std::endl;
    std::cout << "  LMB drag- Rotate camera" << std::endl;
    std::cout << "  C       - Clear all particles" << std::endl;
    std::cout << "  ESC     - Exit" << std::endl;
    std::cout << std::endl;
    
    Simulation sim;
    if (!sim.init(1280, 720, "Atomic Sandbox - Realistic Particle Simulator")) {
        std::cerr << "Failed to initialize simulation" << std::endl;
        return -1;
    }
    
    sim.run();
    std::cout << "Goodbye!" << std::endl;
    return 0;
}