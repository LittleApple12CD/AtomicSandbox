#include "Simulation.hpp"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <random>

Simulation* Simulation::currentInstance = nullptr;

Simulation::Simulation()
    : window(nullptr)
    , windowWidth(1280)
    , windowHeight(720)
    , cameraPos(0.0f, 0.0f, 5.0f)
    , cameraTarget(0.0f, 0.0f, 0.0f)
    , cameraDistance(5.0f)
    , targetCameraDistance(5.0f)
    , cameraYaw(-90.0f)
    , cameraPitch(0.0f)
    , mouseLeftDown(false)
    , lastMouseX(0.0)
    , lastMouseY(0.0)
    , brushRadius(0.1f)
    , brushStrength(1.0f)
    , currentBrushType(ParticleType::PROTON)
    , physicsEngine(std::make_unique<PhysicsEngine>())
    , lodSystem(std::make_unique<LODSystem>(1.0f))
    , timeManager(std::make_unique<TimeManager>())
    , renderer(std::make_unique<Renderer>()) {
}

Simulation::~Simulation() { shutdown(); }

bool Simulation::init(int width, int height, const char* title) {
    std::cout << "Step 1: Starting init..." << std::endl;
    
    windowWidth = width;
    windowHeight = height;

    std::cout << "Step 2: Initializing GLFW..." << std::endl;
    if (!glfwInit()) return false;
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    std::cout << "Step 3: Creating window..." << std::endl;
    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) { glfwTerminate(); return false; }
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    std::cout << "Step 4: Loading GLAD..." << std::endl;
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }
    
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

    std::cout << "Step 5: Initializing renderer..." << std::endl;
    if (!renderer->init()) {
        std::cerr << "Renderer init failed" << std::endl;
        return false;
    }

    std::cout << "Step 6: Setting up OpenGL state..." << std::endl;
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);

    std::cout << "Step 7: Setting up callbacks..." << std::endl;
    setupCallbacks();

    std::cout << "Step 8: Adding test particles..." << std::endl;
    
    // 添加一个质子-电子对，初始距离适当
    addParticle(ParticleType::PROTON, glm::vec3(0.0f, 0.0f, 0.0f));
    addParticle(ParticleType::ELECTRON, glm::vec3(0.15f, 0.0f, 0.0f));
    
    // 给电子一个切向速度（模拟轨道运动）
    if (particles.size() >= 2) {
        particles[1].vel = glm::vec3(0.0f, 2.0f, 0.0f);
    }

    std::cout << "Simulation initialized successfully" << std::endl;
    return true;
}

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

void Simulation::onScroll(double, double yoffset) {
    // 视角缩放
    targetCameraDistance -= yoffset * 0.5f;
    if (targetCameraDistance < 1.0f) targetCameraDistance = 1.0f;
    if (targetCameraDistance > 20.0f) targetCameraDistance = 20.0f;
    std::cout << "Camera distance: " << targetCameraDistance << std::endl;
}

void Simulation::onKey(int key, int, int action, int) {
    if (action != GLFW_PRESS) return;
    switch (key) {
        case GLFW_KEY_LEFT_BRACKET:  // [ 键减慢时间
            timeManager->setTimeScale(timeManager->getTimeScale() * 0.9f);
            std::cout << "Time scale: " << timeManager->getTimeScale() << "x" << std::endl;
            break;
        case GLFW_KEY_RIGHT_BRACKET: // ] 键加快时间
            timeManager->setTimeScale(timeManager->getTimeScale() * 1.1f);
            std::cout << "Time scale: " << timeManager->getTimeScale() << "x" << std::endl;
            break;
        case GLFW_KEY_SPACE:
            if (timeManager->isPaused()) timeManager->resume();
            else timeManager->pause();
            std::cout << (timeManager->isPaused() ? "Paused" : "Resumed") << std::endl;
            break;
        case GLFW_KEY_RIGHT:
            timeManager->stepFrame();
            std::cout << "Frame step" << std::endl;
            break;
        case GLFW_KEY_1: currentBrushType = ParticleType::PROTON; std::cout << "Brush: PROTON\n"; break;
        case GLFW_KEY_2: currentBrushType = ParticleType::NEUTRON; std::cout << "Brush: NEUTRON\n"; break;
        case GLFW_KEY_3: currentBrushType = ParticleType::ELECTRON; std::cout << "Brush: ELECTRON\n"; break;
        case GLFW_KEY_4: currentBrushType = ParticleType::PHOTON; std::cout << "Brush: PHOTON\n"; break;
        case GLFW_KEY_5: currentBrushType = ParticleType::GLUON; std::cout << "Brush: GLUON\n"; break;
        case GLFW_KEY_6: currentBrushType = ParticleType::W_BOSON; std::cout << "Brush: W BOSON\n"; break;
        case GLFW_KEY_7: currentBrushType = ParticleType::UP_QUARK; std::cout << "Brush: UP QUARK\n"; break;
        case GLFW_KEY_8: currentBrushType = ParticleType::DOWN_QUARK; std::cout << "Brush: DOWN QUARK\n"; break;
        case GLFW_KEY_C: clearParticles(); std::cout << "Cleared\n"; break;
        case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, true); break;
    }
}

void Simulation::addParticle(ParticleType type, glm::vec3 position) {
    particles.emplace_back(type, position);
    
    // 给新粒子一个小的随机速度（让运动可见）
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> dis(-0.5, 0.5);
    
    // 质子和中子速度更小
    if (type == ParticleType::PROTON || type == ParticleType::NEUTRON) {
        particles.back().vel = glm::vec3(dis(gen), dis(gen), dis(gen)) * 0.2f;
    } else {
        particles.back().vel = glm::vec3(dis(gen), dis(gen), dis(gen));
    }
    
    std::cout << "Added " << Particle::typeToString(type) << ", total: " << particles.size() << std::endl;
}

void Simulation::clearParticles() {
    particles.clear();
}

void Simulation::updateCamera() {
    // 平滑缩放（lerp 插值）
    cameraDistance += (targetCameraDistance - cameraDistance) * 0.1f;
    
    float radYaw = glm::radians(cameraYaw);
    float radPitch = glm::radians(cameraPitch);
    glm::vec3 offset(cos(radYaw)*cos(radPitch), sin(radPitch), sin(radYaw)*cos(radPitch));
    cameraPos = cameraTarget + offset * cameraDistance;
    renderer->setCamera(cameraPos, cameraTarget);
}

void Simulation::run() {
    double lastTime = glfwGetTime();
    int frameCount = 0;
    
    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        float dt = float(now - lastTime);
        lastTime = now;
        if (dt > 0.033f) dt = 0.033f;

        float simDt = timeManager->getDeltaTime();
        
        // 添加调试：每秒打印一次物理状态
        frameCount++;
        if (frameCount % 60 == 0 && !particles.empty()) {
            std::cout << "Physics: " << particles.size() << " particles, "
                      << "simDt=" << simDt << ", "
                      << "paused=" << timeManager->isPaused() << std::endl;
            // 打印第一个粒子的速度
            std::cout << "  First particle vel: " << particles[0].vel.x 
                      << ", " << particles[0].vel.y << ", " << particles[0].vel.z << std::endl;
        }
        
        if (simDt > 0.0f && !particles.empty()) {
            float dist = glm::distance(cameraPos, cameraTarget);
            auto level = lodSystem->getLevelForDistance(dist);
            if (level == SimulationLevel::SUBATOMIC) {
                physicsEngine->computeForces(particles, simDt);
                physicsEngine->updatePositions(particles, simDt);
            }
        }

        updateCamera();
        renderer->render(particles, glm::distance(cameraPos, cameraTarget), dt);
        handleInput();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Simulation::handleInput() {}
void Simulation::shutdown() {
    if (window) glfwDestroyWindow(window);
    glfwTerminate();
}