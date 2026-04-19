#include "Renderer.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <cstring>

// ==================== 球体顶点数据（使用索引缓冲区） ====================
static std::vector<float> sphereVertices;
static std::vector<unsigned int> sphereIndices;
static int sphereIndexCount = 0;

static void generateSphere(int rings, int sectors) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    float const R = 0.5f;
    
    // 生成顶点
    for (int i = 0; i <= rings; ++i) {
        float phi = 3.14159f * float(i) / float(rings);
        float sinPhi = sin(phi);
        float cosPhi = cos(phi);
        
        for (int j = 0; j <= sectors; ++j) {
            float theta = 2.0f * 3.14159f * float(j) / float(sectors);
            float sinTheta = sin(theta);
            float cosTheta = cos(theta);
            
            float x = R * sinPhi * cosTheta;
            float y = R * sinPhi * sinTheta;
            float z = R * cosPhi;
            
            // 法线就是归一化的位置
            float nx = x / R;
            float ny = y / R;
            float nz = z / R;
            
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            vertices.push_back(nx);
            vertices.push_back(ny);
            vertices.push_back(nz);
        }
    }
    
    // 生成索引（三角形）
    for (int i = 0; i < rings; ++i) {
        for (int j = 0; j < sectors; ++j) {
            int first = i * (sectors + 1) + j;
            int second = first + sectors + 1;
            
            // 第一个三角形
            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);
            
            // 第二个三角形
            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }
    
    sphereVertices = vertices;
    sphereIndices = indices;
    sphereIndexCount = indices.size();
}

// ==================== 构造与析构 ====================
Renderer::Renderer() {
    viewMatrix = glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    projectionMatrix = glm::perspective(glm::radians(45.0f), 1280.0f / 720.0f, 0.01f, 1000.0f);
    generateSphere(24, 24);  // 生成球体
}

Renderer::~Renderer() {
    shutdown();
}

// ==================== 文件加载 ====================
bool Renderer::loadShaderFile(const std::string& filepath, std::string& outSource) {
    std::vector<std::string> searchPaths = {
        filepath,
        "shaders/" + filepath,
        "../shaders/" + filepath,
        "./shaders/" + filepath,
        "D:/AtomicSandbox/shaders/" + filepath,
        "/d/AtomicSandbox/shaders/" + filepath
    };
    
    for (const auto& path : searchPaths) {
        FILE* file = fopen(path.c_str(), "rb");
        if (file) {
            fseek(file, 0, SEEK_END);
            long size = ftell(file);
            fseek(file, 0, SEEK_SET);
            char* buffer = new char[size + 1];
            fread(buffer, 1, size, file);
            buffer[size] = '\0';
            outSource = buffer;
            delete[] buffer;
            fclose(file);
            std::cout << "Loaded: " << path << " (" << size << " bytes)" << std::endl;
            return true;
        }
    }
    std::cerr << "Failed to load: " << filepath << std::endl;
    return false;
}

GLuint Renderer::compileShader(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, log);
        std::cerr << "Shader compile error: " << log << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

// ==================== 主着色器 ====================
bool Renderer::createMainShaderProgram() {
    std::string vertexSrc, fragmentSrc;
    if (!loadShaderFile("vertex.glsl", vertexSrc)) return false;
    if (!loadShaderFile("fragment.glsl", fragmentSrc)) return false;
    
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexSrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentSrc);
    if (vs == 0 || fs == 0) return false;
    
    mainShaderProgram = glCreateProgram();
    glAttachShader(mainShaderProgram, vs);
    glAttachShader(mainShaderProgram, fs);
    glLinkProgram(mainShaderProgram);
    
    GLint success;
    glGetProgramiv(mainShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetProgramInfoLog(mainShaderProgram, 1024, nullptr, log);
        std::cerr << "Main shader link error: " << log << std::endl;
        return false;
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
    
    uViewLoc = glGetUniformLocation(mainShaderProgram, "uView");
    uProjectionLoc = glGetUniformLocation(mainShaderProgram, "uProjection");
    uTimeLoc = glGetUniformLocation(mainShaderProgram, "uTime");
    uCameraPosLoc = glGetUniformLocation(mainShaderProgram, "uCameraPos");
    
    std::cout << "Main shader OK" << std::endl;
    return true;
}

// ==================== 电子云着色器 ====================
bool Renderer::createElectronCloudShaderProgram() {
    std::string vertexSrc, fragSrc, geomSrc;
    if (!loadShaderFile("vertex.glsl", vertexSrc)) return false;
    if (!loadShaderFile("electron_cloud.frag", fragSrc)) return false;
    if (!loadShaderFile("electron_cloud.geom", geomSrc)) return false;
    
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexSrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragSrc);
    GLuint gs = compileShader(GL_GEOMETRY_SHADER, geomSrc);
    if (vs == 0 || fs == 0 || gs == 0) return false;
    
    electronCloudProgram = glCreateProgram();
    glAttachShader(electronCloudProgram, vs);
    glAttachShader(electronCloudProgram, fs);
    glAttachShader(electronCloudProgram, gs);
    glLinkProgram(electronCloudProgram);
    
    GLint success;
    glGetProgramiv(electronCloudProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetProgramInfoLog(electronCloudProgram, 1024, nullptr, log);
        std::cerr << "Electron cloud link error: " << log << std::endl;
        return false;
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
    glDeleteShader(gs);
    
    ecViewLoc = glGetUniformLocation(electronCloudProgram, "uView");
    ecProjectionLoc = glGetUniformLocation(electronCloudProgram, "uProjection");
    ecTimeLoc = glGetUniformLocation(electronCloudProgram, "uTime");
    
    std::cout << "Electron cloud shader OK" << std::endl;
    return true;
}

// ==================== 初始化 ====================
bool Renderer::init() {
    std::cout << "========================================" << std::endl;
    std::cout << "Renderer Initialization" << std::endl;
    std::cout << "========================================" << std::endl;
    
    if (!createMainShaderProgram()) return false;
    if (!createElectronCloudShaderProgram()) {
        std::cout << "Electron cloud disabled, using fallback" << std::endl;
        useElectronCloudShader = false;
    } else {
        useElectronCloudShader = true;
    }
    
    setupSphereGeometry();
    setupInstanceBuffers(20000);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glClearColor(0.05f, 0.05f, 0.12f, 1.0f);
    
    std::cout << "Renderer ready (electron cloud: " << (useElectronCloudShader ? "ON" : "OFF") << ")" << std::endl;
    return true;
}

void Renderer::shutdown() {
    if (sphereVAO) glDeleteVertexArrays(1, &sphereVAO);
    if (sphereVBO) glDeleteBuffers(1, &sphereVBO);
    if (sphereEBO) glDeleteBuffers(1, &sphereEBO);
    if (instanceVAO) glDeleteVertexArrays(1, &instanceVAO);
    if (instanceVBO) glDeleteBuffers(1, &instanceVBO);
    if (mainShaderProgram) glDeleteProgram(mainShaderProgram);
    if (electronCloudProgram) glDeleteProgram(electronCloudProgram);
}

// ==================== 几何体设置（使用索引缓冲区） ====================
void Renderer::setupSphereGeometry() {
    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glGenBuffers(1, &sphereEBO);
    
    glBindVertexArray(sphereVAO);
    
    // 顶点缓冲区
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), sphereVertices.data(), GL_STATIC_DRAW);
    
    // 索引缓冲区
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), sphereIndices.data(), GL_STATIC_DRAW);
    
    // 位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // 法线属性
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
}

void Renderer::setupInstanceBuffers(size_t maxInstances) {
    glGenVertexArrays(1, &instanceVAO);
    glGenBuffers(1, &instanceVBO);
    
    glBindVertexArray(instanceVAO);
    
    // 球体顶点属性
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // 索引缓冲区
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    
    // 实例数据
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, maxInstances * sizeof(ParticleInstance), nullptr, GL_DYNAMIC_DRAW);
    
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleInstance), (void*)offsetof(ParticleInstance, position));
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);
    
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleInstance), (void*)offsetof(ParticleInstance, color));
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);
    
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleInstance), (void*)offsetof(ParticleInstance, size));
    glEnableVertexAttribArray(4);
    glVertexAttribDivisor(4, 1);
    
    glVertexAttribIPointer(5, 1, GL_INT, sizeof(ParticleInstance), (void*)offsetof(ParticleInstance, type));
    glEnableVertexAttribArray(5);
    glVertexAttribDivisor(5, 1);
    
    glBindVertexArray(0);
}

void Renderer::updateInstanceBuffer(const std::vector<ParticleInstance>& instances) {
    if (instances.empty()) return;
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, instances.size() * sizeof(ParticleInstance), instances.data());
}

// ==================== 粒子属性 ====================
glm::vec4 Renderer::getParticleColor(ParticleType type, float intensity) {
    switch (type) {
        case ParticleType::PROTON:     return glm::vec4(0.95f, 0.35f, 0.25f, intensity);
        case ParticleType::NEUTRON:    return glm::vec4(0.65f, 0.68f, 0.78f, intensity);
        case ParticleType::ELECTRON:   return glm::vec4(0.25f, 0.65f, 1.00f, intensity);
        case ParticleType::PHOTON:     return glm::vec4(1.00f, 0.92f, 0.45f, intensity);
        case ParticleType::GLUON:      return glm::vec4(0.75f, 0.45f, 0.95f, intensity);
        case ParticleType::W_BOSON:    return glm::vec4(0.92f, 0.28f, 0.28f, intensity);
        case ParticleType::UP_QUARK:   return glm::vec4(0.25f, 0.88f, 0.42f, intensity);
        case ParticleType::DOWN_QUARK: return glm::vec4(0.88f, 0.55f, 0.25f, intensity);
        default: return glm::vec4(1.0f, 1.0f, 1.0f, intensity);
    }
}

float Renderer::getParticleSize(ParticleType type, float viewDistance) {
    float baseSize = 0.12f;
    if (type == ParticleType::PROTON) baseSize = 0.15f;
    if (type == ParticleType::NEUTRON) baseSize = 0.14f;
    if (type == ParticleType::ELECTRON) baseSize = 0.10f;
    if (type == ParticleType::PHOTON) baseSize = 0.08f;
    if (type == ParticleType::GLUON) baseSize = 0.09f;
    float scale = std::max(0.4f, 4.0f / (viewDistance + 1.0f));
    return baseSize * scale;
}

// ==================== 渲染 ====================
void Renderer::renderParticlesWithMainShader(const std::vector<ParticleInstance>& instances) {
    if (instances.empty()) return;
    updateInstanceBuffer(instances);
    glUseProgram(mainShaderProgram);
    glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(uProjectionLoc, 1, GL_FALSE, &projectionMatrix[0][0]);
    glUniform1f(uTimeLoc, timeAccumulator);
    glm::vec3 camPos = glm::vec3(glm::inverse(viewMatrix)[3]);
    glUniform3fv(uCameraPosLoc, 1, &camPos[0]);
    glBindVertexArray(instanceVAO);
    glDrawElementsInstanced(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0, (GLsizei)instances.size());
}

void Renderer::renderElectronsWithCloudShader(const std::vector<ParticleInstance>& instances) {
    if (instances.empty()) return;
    updateInstanceBuffer(instances);
    glUseProgram(electronCloudProgram);
    glUniformMatrix4fv(ecViewLoc, 1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(ecProjectionLoc, 1, GL_FALSE, &projectionMatrix[0][0]);
    glUniform1f(ecTimeLoc, timeAccumulator);
    glBindVertexArray(instanceVAO);
    glDrawElementsInstanced(GL_POINTS, sphereIndexCount, GL_UNSIGNED_INT, 0, (GLsizei)instances.size());
}

void Renderer::render(const std::vector<Particle>& particles, float viewDistance, float deltaTime) {
    if (particles.empty()) return;
    if (mainShaderProgram == 0) return;
    
    timeAccumulator += deltaTime;
    
    std::vector<ParticleInstance> instances;
    instances.reserve(particles.size());
    for (const auto& p : particles) {
        if (!p.active) continue;
        instances.push_back({p.pos, getParticleColor(p.type, 1.0f), getParticleSize(p.type, viewDistance), (int)p.type});
    }
    if (instances.empty()) return;
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // 分离电子
    std::vector<ParticleInstance> electrons, others;
    for (const auto& inst : instances) {
        if (inst.type == (int)ParticleType::ELECTRON) electrons.push_back(inst);
        else others.push_back(inst);
    }
    
    // 先渲染其他粒子
    if (!others.empty()) renderParticlesWithMainShader(others);
    
    // 用电子云着色器渲染电子
    if (useElectronCloudShader && electronCloudProgram != 0 && !electrons.empty()) {
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        renderElectronsWithCloudShader(electrons);
        glDepthMask(GL_TRUE);
    } else if (!electrons.empty()) {
        renderParticlesWithMainShader(electrons);
    }
}

void Renderer::setCamera(const glm::vec3& position, const glm::vec3& target) {
    viewMatrix = glm::lookAt(position, target, glm::vec3(0, 1, 0));
}

void Renderer::onResize(int width, int height) {
    screenWidth = width;
    screenHeight = height;
    glViewport(0, 0, width, height);
    projectionMatrix = glm::perspective(glm::radians(45.0f), (float)width / height, 0.01f, 1000.0f);
}