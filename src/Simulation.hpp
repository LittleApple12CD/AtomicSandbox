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

    // 粒子管理
    void addParticle(ParticleType type, glm::vec3 position);
    void clearParticles();
    
    // 画笔控制
    void setBrushRadius(float radius) { brushRadius = radius; }
    void setBrushStrength(float strength) { brushStrength = strength; }
    void setBrushType(ParticleType type) { currentBrushType = type; }

private:
    // ==================== 核心系统 ====================
    std::vector<Particle> particles;
    std::unique_ptr<PhysicsEngine> physicsEngine;
    std::unique_ptr<LODSystem> lodSystem;
    std::unique_ptr<TimeManager> timeManager;
    std::unique_ptr<Renderer> renderer;
    
    // ==================== 窗口 ====================
    GLFWwindow* window;
    int windowWidth;
    int windowHeight;
    
    // ==================== 相机控制 ====================
    glm::vec3 cameraPos;
    glm::vec3 cameraTarget;
    float cameraDistance;
    float targetCameraDistance;
    float cameraYaw;
    float cameraPitch;
    bool mouseLeftDown;
    double lastMouseX;
    double lastMouseY;
    
    // ==================== 画笔 ====================
    float brushRadius;
    float brushStrength;
    ParticleType currentBrushType;
    
    // ==================== 输入处理 ====================
    void handleInput();
    void updateCamera();
    void setupCallbacks();
    void spawnParticleAtMouse();
    void printControls();
    void printDebugInfo();
    
    // ==================== 静态回调函数 ====================
    static Simulation* currentInstance;
    static void mouseCallback(GLFWwindow* win, double xpos, double ypos);
    static void mouseButtonCallback(GLFWwindow* win, int button, int action, int mods);
    static void scrollCallback(GLFWwindow* win, double xoffset, double yoffset);
    static void keyCallback(GLFWwindow* win, int key, int scancode, int action, int mods);
    
    // ==================== 实例回调函数 ====================
    void onMouseMove(double xpos, double ypos);
    void onMouseButton(int button, int action, int mods);
    void onScroll(double xoffset, double yoffset);
    void onKey(int key, int scancode, int action, int mods);
    
    // ==================== 调试 ====================
    float frameTimeAccumulator;
    int frameCount;
};