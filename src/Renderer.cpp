#include "Renderer.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <cstdio>
#include <vector>
#include <algorithm>

static const float sphereVertices[] = {
    -0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,
     0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f,
     0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,
     0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f
};
static const int sphereVertexCount = sizeof(sphereVertices) / (3 * sizeof(float));

Renderer::Renderer() {
    viewMatrix = glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    projectionMatrix = glm::perspective(glm::radians(45.0f), 1280.0f / 720.0f, 0.01f, 1000.0f);
}

Renderer::~Renderer() { shutdown(); }

bool Renderer::init() {
    std::cout << "Renderer: Compiling shaders..." << std::endl;
    if (!compileShaders()) return false;
    
    std::cout << "Renderer: Setting up sphere geometry..." << std::endl;
    setupSphereGeometry();
    
    std::cout << "Renderer: Setting up instance buffers..." << std::endl;
    setupInstanceBuffers(20000);
    
    std::cout << "Renderer: Enabling OpenGL features..." << std::endl;
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.05f, 0.05f, 0.12f, 1.0f);
    
    std::cout << "Renderer initialized successfully" << std::endl;
    return true;
}

void Renderer::shutdown() {
    if (sphereVAO) glDeleteVertexArrays(1, &sphereVAO);
    if (sphereVBO) glDeleteBuffers(1, &sphereVBO);
    if (instanceVAO) glDeleteVertexArrays(1, &instanceVAO);
    if (instanceVBO) glDeleteBuffers(1, &instanceVBO);
    if (shaderProgram) glDeleteProgram(shaderProgram);
    if (electronCloudProgram) glDeleteProgram(electronCloudProgram);
}

bool Renderer::compileShaders() {
    // 主着色器（所有粒子都用这个，包括电子）
    std::string vertexSrc, fragmentSrc;
    if (!loadShaderSource("vertex.glsl", vertexSrc)) return false;
    if (!loadShaderSource("fragment.glsl", fragmentSrc)) return false;
    
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexSrc.c_str());
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentSrc.c_str());
    
    if (vs == 0 || fs == 0) return false;
    
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);
    
    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, log);
        std::cerr << "Shader link error: " << log << std::endl;
        return false;
    }
    
    glDeleteShader(vs);
    glDeleteShader(fs);
    
    uViewLoc = glGetUniformLocation(shaderProgram, "uView");
    uProjectionLoc = glGetUniformLocation(shaderProgram, "uProjection");
    uTimeLoc = glGetUniformLocation(shaderProgram, "uTime");
    uCameraPosLoc = glGetUniformLocation(shaderProgram, "uCameraPos");
    
    std::cout << "Main shader compiled successfully" << std::endl;
    
    // 暂时禁用电子云着色器（让电子先用球体渲染，确保能看见）
    renderElectronClouds = true;
    
    return true;
}

GLuint Renderer::compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "Shader compile error: " << log << std::endl;
    }
    return shader;
}

bool Renderer::loadShaderSource(const char* filepath, std::string& outSource) {
    std::string fullPath = "shaders/";
    fullPath += filepath;
    
    FILE* file = fopen(fullPath.c_str(), "rb");
    if (!file) {
        fullPath = "../shaders/";
        fullPath += filepath;
        file = fopen(fullPath.c_str(), "rb");
    }
    
    if (!file) {
        std::cerr << "Cannot open shader: " << filepath << std::endl;
        return false;
    }
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* buffer = new char[size + 1];
    fread(buffer, 1, size, file);
    buffer[size] = '\0';
    outSource = buffer;
    delete[] buffer;
    fclose(file);
    
    return true;
}

void Renderer::setupSphereGeometry() {
    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glBindVertexArray(sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphereVertices), sphereVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void Renderer::setupInstanceBuffers(size_t maxInstances) {
    glGenVertexArrays(1, &instanceVAO);
    glGenBuffers(1, &instanceVBO);
    glBindVertexArray(instanceVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, maxInstances * sizeof(ParticleInstance), nullptr, GL_DYNAMIC_DRAW);
    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleInstance), (void*)offsetof(ParticleInstance, position));
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);
    
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleInstance), (void*)offsetof(ParticleInstance, color));
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);
    
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleInstance), (void*)offsetof(ParticleInstance, size));
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);
    
    glVertexAttribIPointer(4, 1, GL_INT, sizeof(ParticleInstance), (void*)offsetof(ParticleInstance, type));
    glEnableVertexAttribArray(4);
    glVertexAttribDivisor(4, 1);
}

void Renderer::updateInstanceBuffer(const std::vector<ParticleInstance>& instances) {
    if (instances.empty()) return;
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, instances.size() * sizeof(ParticleInstance), instances.data());
}

glm::vec4 Renderer::getParticleColor(ParticleType type, float intensity) {
    switch (type) {
        case ParticleType::PROTON:     return glm::vec4(0.95f, 0.35f, 0.25f, intensity);
        case ParticleType::NEUTRON:    return glm::vec4(0.65f, 0.65f, 0.75f, intensity);
        case ParticleType::ELECTRON:   return glm::vec4(0.20f, 0.60f, 1.00f, intensity);
        case ParticleType::PHOTON:     return glm::vec4(1.00f, 0.95f, 0.50f, intensity);
        case ParticleType::GLUON:      return glm::vec4(0.70f, 0.40f, 0.90f, intensity);
        case ParticleType::W_BOSON:    return glm::vec4(0.90f, 0.25f, 0.25f, intensity);
        case ParticleType::Z_BOSON:    return glm::vec4(0.25f, 0.25f, 0.90f, intensity);
        case ParticleType::UP_QUARK:   return glm::vec4(0.20f, 0.85f, 0.35f, intensity);
        case ParticleType::DOWN_QUARK: return glm::vec4(0.85f, 0.55f, 0.25f, intensity);
        default: return glm::vec4(1.0f, 1.0f, 1.0f, intensity);
    }
}

float Renderer::getParticleSize(ParticleType type, float viewDistance) {
    float baseSize = 0.025f;
    if (type == ParticleType::ELECTRON) baseSize = 0.020f;  // 电子稍微大一点，更容易看见
    if (type == ParticleType::PHOTON) baseSize = 0.010f;
    if (type == ParticleType::GLUON) baseSize = 0.012f;
    
    float scale = std::max(0.3f, 3.0f / (viewDistance + 1.0f));
    return baseSize * scale;
}

void Renderer::render(const std::vector<Particle>& particles, float viewDistance, float deltaTime) {
    if (particles.empty()) return;
    if (shaderProgram == 0) return;
    
    timeAccumulator += deltaTime;
    
    // 构建实例数据
    std::vector<ParticleInstance> instances;
    instances.reserve(particles.size());
    for (const auto& p : particles) {
        if (!p.active) continue;
        instances.push_back({
            p.pos,
            getParticleColor(p.type, 1.0f),
            getParticleSize(p.type, viewDistance),
            static_cast<int>(p.type)
        });
    }
    
    if (instances.empty()) return;
    
    updateInstanceBuffer(instances);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glUseProgram(shaderProgram);
    
    glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(uProjectionLoc, 1, GL_FALSE, &projectionMatrix[0][0]);
    glUniform1f(uTimeLoc, timeAccumulator);
    
    glm::vec3 camPos = glm::vec3(glm::inverse(viewMatrix)[3]);
    glUniform3fv(uCameraPosLoc, 1, &camPos[0]);
    
    glBindVertexArray(instanceVAO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, sphereVertexCount, (GLsizei)instances.size());
}

void Renderer::setCamera(const glm::vec3& position, const glm::vec3& target) {
    viewMatrix = glm::lookAt(position, target, glm::vec3(0,1,0));
}

void Renderer::onResize(int width, int height) {
    screenWidth = width;
    screenHeight = height;
    glViewport(0, 0, width, height);
    projectionMatrix = glm::perspective(glm::radians(45.0f), (float)width/height, 0.01f, 1000.0f);
}