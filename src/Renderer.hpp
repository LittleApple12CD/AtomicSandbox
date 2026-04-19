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

    const glm::mat4& getViewMatrix() const { return viewMatrix; }
    const glm::mat4& getProjectionMatrix() const { return projectionMatrix; }

private:
    // 着色器程序
    GLuint mainShaderProgram = 0;
    GLuint electronCloudProgram = 0;
    
    // Uniform 位置
    int uViewLoc = -1;
    int uProjectionLoc = -1;
    int uTimeLoc = -1;
    int uCameraPosLoc = -1;
    
    int ecViewLoc = -1;
    int ecProjectionLoc = -1;
    int ecTimeLoc = -1;
    
    // OpenGL 资源
    GLuint sphereVAO = 0;
    GLuint sphereVBO = 0;
    GLuint instanceVBO = 0;
    GLuint instanceVAO = 0;
    GLuint sphereEBO = 0;
    
    // 矩阵
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    
    // 状态
    int screenWidth = 1280;
    int screenHeight = 720;
    float timeAccumulator = 0.0f;
    bool useElectronCloudShader = true;
    
    // 着色器加载和编译
    bool loadShaderFile(const std::string& filepath, std::string& outSource);
    GLuint compileShader(GLenum type, const std::string& source);
    bool createMainShaderProgram();
    bool createElectronCloudShaderProgram();
    
    // 几何体设置
    void setupSphereGeometry();
    void setupInstanceBuffers(size_t maxInstances);
    void updateInstanceBuffer(const std::vector<ParticleInstance>& instances);
    
    // 粒子属性
    glm::vec4 getParticleColor(ParticleType type, float intensity = 1.0f);
    float getParticleSize(ParticleType type, float viewDistance);
    
    // 渲染
    void renderParticlesWithMainShader(const std::vector<ParticleInstance>& instances);
    void renderElectronsWithCloudShader(const std::vector<ParticleInstance>& instances);
};