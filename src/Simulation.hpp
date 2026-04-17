#pragma once

#include <vector>
#include <memory>
#include <GLFW/glfw3.h>
#include "Particle.hpp"
#include "PhysicsEngine.hpp"
#include "LODSystem.hpp"
#include "TimeManager.hpp"
#include "Renderer.hpp"

class Simulation {
public:
    Simulation();
    ~Simulation();

    bool init(int width, int height, const char* title);
    void run();
    void shutdown();

    void addParticle(ParticleType type, glm::vec3 position);
    void clearParticles();

private:
    void handleInput();
    void updateCamera();
    void setupCallbacks();
    void spawnParticleAtMouse();

    static void mouseCallback(GLFWwindow* win, double xpos, double ypos);
    static void mouseButtonCallback(GLFWwindow* win, int button, int action, int mods);
    static void scrollCallback(GLFWwindow* win, double xoffset, double yoffset);
    static void keyCallback(GLFWwindow* win, int key, int scancode, int action, int mods);
    void onMouseMove(double xpos, double ypos);
    void onMouseButton(int button, int action, int mods);
    void onScroll(double xoffset, double yoffset);
    void onKey(int key, int scancode, int action, int mods);

    std::vector<Particle> particles;
    std::unique_ptr<PhysicsEngine> physicsEngine;
    std::unique_ptr<LODSystem> lodSystem;
    std::unique_ptr<TimeManager> timeManager;
    std::unique_ptr<Renderer> renderer;

    GLFWwindow* window = nullptr;
    int windowWidth = 1280, windowHeight = 720;

    glm::vec3 cameraPos = glm::vec3(0,0,5);
    glm::vec3 cameraTarget = glm::vec3(0,0,0);
    float cameraDistance = 5.0f;
    float cameraYaw = -90.0f;
    float cameraPitch = 0.0f;
    bool mouseLeftDown = false;
    double lastMouseX = 0.0, lastMouseY = 0.0;
    
    float targetCameraDistance;  // 目标相机距离（用于平滑缩放）
    float brushRadius = 0.1f;
    float brushStrength = 1.0f;
    ParticleType currentBrushType = ParticleType::PROTON;

    static Simulation* currentInstance;
};