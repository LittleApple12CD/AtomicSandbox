#include "Simulation.hpp"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <random>

Simulation* Simulation::currentInstance = nullptr;

// ==================== 构造函数 ====================
Simulation::Simulation()
    : window(nullptr)
    , windowWidth(1280)
    , windowHeight(720)
    , cameraPos(0.0f, 0.0f, 12.0f)
    , cameraTarget(0.0f, 0.0f, 0.0f)
    , cameraDistance(12.0f)
    , targetCameraDistance(12.0f)
    , cameraYaw(-90.0f)
    , cameraPitch(25.0f)
    , mouseLeftDown(false)
    , lastMouseX(0.0)
    , lastMouseY(0.0)
    , brushRadius(0.2f)
    , brushStrength(1.0f)
    , currentBrushType(ParticleType::PROTON)
    , frameTimeAccumulator(0.0f)
    , frameCount(0) {
    
    physicsEngine = std::make_unique<PhysicsEngine>();
    lodSystem = std::make_unique<LODSystem>(1.0f);
    timeManager = std::make_unique<TimeManager>();
    renderer = std::make_unique<Renderer>();
    
    std::cout << "Simulation constructed" << std::endl;
}

Simulation::~Simulation() {
    shutdown();
}

// ==================== 打印控制说明 ====================
void Simulation::printControls() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "Atomic Sandbox - Rule-Based Physics" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "\n=== Camera Controls ===" << std::endl;
    std::cout << "  LMB drag    - Rotate camera" << std::endl;
    std::cout << "  Scroll      - Zoom in/out" << std::endl;
    std::cout << "\n=== Time Controls ===" << std::endl;
    std::cout << "  Space       - Pause/Resume" << std::endl;
    std::cout << "  Right Arrow - Single frame step" << std::endl;
    std::cout << "  [ / ]       - Slow down / Speed up time" << std::endl;
    std::cout << "  \\           - Reset time scale to 1x" << std::endl;
    std::cout << "\n=== Particle Brush ===" << std::endl;
    std::cout << "  1 - Proton     2 - Neutron     3 - Electron" << std::endl;
    std::cout << "  4 - Photon     5 - Gluon       6 - W Boson" << std::endl;
    std::cout << "  7 - Z Boson    8 - Up Quark    9 - Down Quark" << std::endl;
    std::cout << "  0 - Neutrino" << std::endl;
    std::cout << "  RMB         - Add particle at cursor" << std::endl;
    std::cout << "  C           - Clear all particles" << std::endl;
    std::cout << "\n=== Physics Rules ===" << std::endl;
    std::cout << "  • 2+ nucleons close → form nucleus" << std::endl;
    std::cout << "  • Electron near nucleus → captured to orbit" << std::endl;
    std::cout << "  • Unstable nuclei → decay/fission" << std::endl;
    std::cout << "  • Nuclei close → fusion" << std::endl;
    std::cout << "\n=== Other ===" << std::endl;
    std::cout << "  ESC         - Exit" << std::endl;
    std::cout << "========================================\n" << std::endl;
}

// ==================== 打印调试信息 ====================
void Simulation::printDebugInfo() {
    const auto& nuclei = physicsEngine->getNuclei();
    int totalElectrons = 0;
    for (const auto& pair : nuclei) {
        totalElectrons += pair.second.electronIndices.size();
    }
    
    std::cout << "\n=== Debug Info ===" << std::endl;
    std::cout << "  Particles: " << particles.size() << std::endl;
    std::cout << "  Nuclei: " << nuclei.size() << std::endl;
    std::cout << "  Electrons in orbits: " << totalElectrons << std::endl;
    std::cout << "  Time scale: " << timeManager->getTimeScale() << "x" << std::endl;
    std::cout << "  Paused: " << (timeManager->isPaused() ? "Yes" : "No") << std::endl;
    std::cout << "=================" << std::endl;
}

// ==================== 初始化 ====================
bool Simulation::init(int width, int height, const char* title) {
    std::cout << "========================================" << std::endl;
    std::cout << "Atomic Sandbox Initialization" << std::endl;
    std::cout << "========================================" << std::endl;
    
    windowWidth = width;
    windowHeight = height;

    // 1. 初始化 GLFW
    std::cout << "[1/6] Initializing GLFW..." << std::endl;
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 2. 创建窗口
    std::cout << "[2/6] Creating window..." << std::endl;
    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window" << std::endl;
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // 3. 加载 GLAD
    std::cout << "[3/6] Loading GLAD..." << std::endl;
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }
    
    std::cout << "  OpenGL version: " << glGetString(GL_VERSION) << std::endl;

    // 4. 初始化渲染器
    std::cout << "[4/6] Initializing renderer..." << std::endl;
    if (!renderer->init()) {
        std::cerr << "Renderer initialization failed" << std::endl;
        return false;
    }

    // 5. 设置 OpenGL 状态
    std::cout << "[5/6] Setting up OpenGL state..." << std::endl;
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.02f, 0.02f, 0.05f, 1.0f);

    // 6. 设置回调和测试粒子
    std::cout << "[6/6] Setting up callbacks and test particles..." << std::endl;
    setupCallbacks();
    
    // 打印控制说明
    printControls();
    
    // 添加测试：两个质子和一个电子
    addParticle(ParticleType::PROTON, glm::vec3(-0.3f, 0.0f, 0.0f));
    addParticle(ParticleType::PROTON, glm::vec3(0.3f, 0.0f, 0.0f));
    addParticle(ParticleType::ELECTRON, glm::vec3(0.0f, 2.5f, 0.0f)); // 距离2.5，会被第一层捕获
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "Initialization Complete!" << std::endl;
    std::cout << "Total particles: " << particles.size() << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    return true;
}

// ==================== 回调设置 ====================
void Simulation::setupCallbacks() {
    currentInstance = this;
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, keyCallback);
}

void Simulation::mouseCallback(GLFWwindow*, double xpos, double ypos) {
    if (currentInstance) currentInstance->onMouseMove(xpos, ypos);
}

void Simulation::mouseButtonCallback(GLFWwindow*, int button, int action, int mods) {
    if (currentInstance) currentInstance->onMouseButton(button, action, mods);
}

void Simulation::scrollCallback(GLFWwindow*, double xoffset, double yoffset) {
    if (currentInstance) currentInstance->onScroll(xoffset, yoffset);
}

void Simulation::keyCallback(GLFWwindow*, int key, int scancode, int action, int mods) {
    if (currentInstance) currentInstance->onKey(key, scancode, action, mods);
}

// ==================== 鼠标事件 ====================
void Simulation::onMouseMove(double xpos, double ypos) {
    if (mouseLeftDown) {
        float dx = xpos - lastMouseX;
        float dy = ypos - lastMouseY;
        cameraYaw += dx * 0.5f;
        cameraPitch -= dy * 0.5f;
        
        if (cameraPitch > 89.0f) cameraPitch = 89.0f;
        if (cameraPitch < -89.0f) cameraPitch = -89.0f;
        
        lastMouseX = xpos;
        lastMouseY = ypos;
    }
}

void Simulation::onMouseButton(int button, int action, int) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            mouseLeftDown = true;
            glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
        } else if (action == GLFW_RELEASE) {
            mouseLeftDown = false;
        }
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        spawnParticleAtMouse();
    }
}

// ==================== 鼠标位置生成粒子 ====================
void Simulation::spawnParticleAtMouse() {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    
    float ndcX = (2.0f * xpos) / windowWidth - 1.0f;
    float ndcY = 1.0f - (2.0f * ypos) / windowHeight;
    
    glm::vec4 rayClip(ndcX, ndcY, -1.0f, 1.0f);
    glm::vec4 rayEye = glm::inverse(renderer->getProjectionMatrix()) * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);
    glm::vec3 rayDir = glm::normalize(glm::vec3(glm::inverse(renderer->getViewMatrix()) * rayEye));
    
    float t = -cameraPos.z / rayDir.z;
    glm::vec3 worldPos = cameraPos + rayDir * t;
    
    addParticle(currentBrushType, worldPos);
}

// ==================== 滚轮事件 ====================
void Simulation::onScroll(double, double yoffset) {
    targetCameraDistance -= yoffset * 0.5f;
    if (targetCameraDistance < 3.0f) targetCameraDistance = 3.0f;
    if (targetCameraDistance > 30.0f) targetCameraDistance = 30.0f;
}

// ==================== 键盘事件 ====================
void Simulation::onKey(int key, int, int action, int) {
    if (action != GLFW_PRESS) return;
    
    switch (key) {
        // 时间控制
        case GLFW_KEY_SPACE:
            if (timeManager->isPaused()) timeManager->resume();
            else timeManager->pause();
            std::cout << (timeManager->isPaused() ? "Paused" : "Resumed") << std::endl;
            break;
        case GLFW_KEY_RIGHT:
            timeManager->stepFrame();
            std::cout << "Frame step" << std::endl;
            break;
        case GLFW_KEY_LEFT_BRACKET:
            timeManager->setTimeScale(timeManager->getTimeScale() * 0.9f);
            std::cout << "Time scale: " << timeManager->getTimeScale() << "x" << std::endl;
            break;
        case GLFW_KEY_RIGHT_BRACKET:
            timeManager->setTimeScale(timeManager->getTimeScale() * 1.1f);
            std::cout << "Time scale: " << timeManager->getTimeScale() << "x" << std::endl;
            break;
        case GLFW_KEY_BACKSLASH:
            timeManager->setTimeScale(1.0f);
            std::cout << "Time scale reset to 1.0x" << std::endl;
            break;
            
        // 画笔类型
        case GLFW_KEY_1: currentBrushType = ParticleType::PROTON; 
            std::cout << "Brush: PROTON" << std::endl; break;
        case GLFW_KEY_2: currentBrushType = ParticleType::NEUTRON; 
            std::cout << "Brush: NEUTRON" << std::endl; break;
        case GLFW_KEY_3: currentBrushType = ParticleType::ELECTRON; 
            std::cout << "Brush: ELECTRON" << std::endl; break;
        case GLFW_KEY_4: currentBrushType = ParticleType::PHOTON; 
            std::cout << "Brush: PHOTON" << std::endl; break;
        case GLFW_KEY_5: currentBrushType = ParticleType::GLUON; 
            std::cout << "Brush: GLUON" << std::endl; break;
        case GLFW_KEY_6: currentBrushType = ParticleType::W_BOSON; 
            std::cout << "Brush: W BOSON" << std::endl; break;
        case GLFW_KEY_7: currentBrushType = ParticleType::Z_BOSON; 
            std::cout << "Brush: Z BOSON" << std::endl; break;
        case GLFW_KEY_8: currentBrushType = ParticleType::UP_QUARK; 
            std::cout << "Brush: UP QUARK" << std::endl; break;
        case GLFW_KEY_9: currentBrushType = ParticleType::DOWN_QUARK; 
            std::cout << "Brush: DOWN QUARK" << std::endl; break;
        case GLFW_KEY_0: currentBrushType = ParticleType::ELECTRON_NEUTRINO; 
            std::cout << "Brush: NEUTRINO" << std::endl; break;
            
        // 清除粒子
        case GLFW_KEY_C:
            clearParticles();
            std::cout << "Cleared all particles" << std::endl;
            break;
            
        // 调试信息
        case GLFW_KEY_D:
            printDebugInfo();
            break;
            
        // 退出
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, true);
            break;
            
        default:
            break;
    }
}

// ==================== 粒子管理 ====================
void Simulation::addParticle(ParticleType type, glm::vec3 position) {
    particles.emplace_back(type, position);
    
    // 给新粒子随机初速度
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> dis(-1.0f, 1.0f);
    
    switch (type) {
        case ParticleType::PROTON:
        case ParticleType::NEUTRON:
            particles.back().vel = glm::vec3(dis(gen), dis(gen), dis(gen)) * 0.5f;
            break;
        case ParticleType::ELECTRON:
            particles.back().vel = glm::vec3(dis(gen), dis(gen), dis(gen)) * 1.2f;
            break;
        default:
            particles.back().vel = glm::vec3(dis(gen), dis(gen), dis(gen)) * 0.8f;
            break;
    }
    
    std::cout << "Added " << Particle::typeToString(type) 
              << " at (" << position.x << ", " << position.y << ", " << position.z << ")" 
              << ", total: " << particles.size() << std::endl;
}

void Simulation::clearParticles() {
    particles.clear();
    physicsEngine->clearNuclei();
    std::cout << "All particles and nuclei cleared" << std::endl;
}

// ==================== 相机更新 ====================
void Simulation::updateCamera() {
    cameraDistance += (targetCameraDistance - cameraDistance) * 0.1f;
    
    float radYaw = glm::radians(cameraYaw);
    float radPitch = glm::radians(cameraPitch);
    
    glm::vec3 offset;
    offset.x = cos(radYaw) * cos(radPitch);
    offset.y = sin(radPitch);
    offset.z = sin(radYaw) * cos(radPitch);
    
    cameraPos = cameraTarget + offset * cameraDistance;
    renderer->setCamera(cameraPos, cameraTarget);
}

// ==================== 主循环 ====================
void Simulation::run() {
    double lastTime = glfwGetTime();
    
    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        float dt = float(now - lastTime);
        lastTime = now;
        if (dt > 0.033f) dt = 0.033f;

        float simDt = timeManager->getDeltaTime();
        
        // 物理更新（基于规则）
        if (simDt > 0.0f && !particles.empty()) {
            float dist = glm::distance(cameraPos, cameraTarget);
            auto level = lodSystem->getLevelForDistance(dist);
            if (level == SimulationLevel::SUBATOMIC) {
                physicsEngine->update(particles, simDt);
            }
        }
        
        // 定期打印状态（每600帧）
        frameCount++;
        if (frameCount % 600 == 0 && !timeManager->isPaused()) {
            // 可选：取消注释以打印调试信息
            // printDebugInfo();
        }

        // 渲染
        updateCamera();
        renderer->render(particles, glm::distance(cameraPos, cameraTarget), dt);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Simulation::shutdown() {
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
    std::cout << "Simulation shutdown complete" << std::endl;
}