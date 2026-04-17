#pragma once

#include <string>
#include <vector>
#include <memory>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Particle.hpp"

struct ParticleInstance {
    glm::vec3 position;
    glm::vec4 color;
    float size;
    int type;
};

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool init();
    void shutdown();
    void render(const std::vector<Particle>& particles, float viewDistance, float deltaTime);
    void setCamera(const glm::vec3& position, const glm::vec3& target);
    void onResize(int width, int height);

    // Getter for ray picking
    const glm::mat4& getViewMatrix() const { return viewMatrix; }
    const glm::mat4& getProjectionMatrix() const { return projectionMatrix; }

    void setRenderElectronClouds(bool enable) { renderElectronClouds = enable; }

private:
    // OpenGL 资源
    GLuint sphereVAO = 0;
    GLuint sphereVBO = 0;
    GLuint instanceVBO = 0;
    GLuint instanceVAO = 0;
    GLuint shaderProgram = 0;
    GLuint electronCloudProgram = 0;
    
    // Uniform 位置（使用 int 以便与 -1 比较）
    int uViewLoc = -1;
    int uProjectionLoc = -1;
    int uTimeLoc = -1;
    int uCameraPosLoc = -1;
    
    int ecViewLoc = -1;
    int ecProjectionLoc = -1;
    int ecTimeLoc = -1;
    
    // 矩阵
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    
    // 窗口尺寸
    int screenWidth = 1280;
    int screenHeight = 720;
    
    // 渲染选项
    bool renderElectronClouds = true;
    float timeAccumulator = 0.0f;
    
    // 着色器辅助函数
    bool compileShaders();
    bool compileMainShader();
    bool compileElectronCloudShader();
    GLuint compileShader(GLenum type, const char* source);
    bool loadShaderSource(const char* filepath, std::string& outSource);
    
    // 几何体设置
    void setupSphereGeometry();
    void setupInstanceBuffers(size_t maxInstances);
    void updateInstanceBuffer(const std::vector<ParticleInstance>& instances);
    
    // 粒子属性
    glm::vec4 getParticleColor(ParticleType type, float intensity = 1.0f);
    float getParticleSize(ParticleType type, float viewDistance);
    
    // 渲染辅助
    void renderParticlesWithMainShader(const std::vector<ParticleInstance>& instances);
    void renderElectronsWithCloudShader(const std::vector<ParticleInstance>& instances);
};