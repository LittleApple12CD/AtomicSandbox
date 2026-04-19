#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aInstancePos;
layout(location = 3) in vec4 aInstanceColor;
layout(location = 4) in float aInstanceSize;
layout(location = 5) in int aInstanceType;

uniform mat4 uView;
uniform mat4 uProjection;
uniform float uTime;
uniform vec3 uCameraPos;

out vec4 vColor;
out float vAlpha;
out float vDistanceToCamera;
out float vParticleType;
out vec3 vViewPos;
out vec3 vNormal;
out vec3 vWorldPos;
out float vGlowIntensity;
out float vSpecularIntensity;

// 随机函数
float random(vec3 seed) {
    return fract(sin(dot(seed, vec3(12.9898, 78.233, 37.719))) * 43758.5453);
}

void main() {
    vParticleType = float(aInstanceType);
    
    // 电子：直接输出位置，不进行球体变换
    if (aInstanceType == 2) {
        vec4 worldPos = vec4(aInstancePos, 1.0);
        vWorldPos = aInstancePos;
        vec4 viewPos = uView * worldPos;
        vViewPos = viewPos.xyz;
        vDistanceToCamera = length(viewPos.xyz);
        gl_Position = uProjection * viewPos;
        vColor = aInstanceColor;
        vAlpha = 0.0;
        vNormal = aNormal;
        vGlowIntensity = 0.0;
        vSpecularIntensity = 0.0;
        return;
    }
    
    // 非电子：正常球体渲染
    vec3 scaledPos = aPos * aInstanceSize;
    vec4 worldPos = vec4(aInstancePos + scaledPos, 1.0);
    vWorldPos = worldPos.xyz;
    
    vec4 viewPos = uView * worldPos;
    vViewPos = viewPos.xyz;
    vDistanceToCamera = length(viewPos.xyz);
    
    vNormal = normalize(aNormal);
    
    // 透明度
    float alpha = 1.0;
    if (vDistanceToCamera > 2.0) {
        alpha = 2.0 / vDistanceToCamera;
    }
    vAlpha = clamp(alpha, 0.6, 1.0);
    
    // 大小缩放
    float distanceScale = 1.0;
    if (vDistanceToCamera > 1.0) {
        distanceScale = 1.0 / (vDistanceToCamera * 0.3);
    }
    distanceScale = clamp(distanceScale, 0.4, 1.5);
    float finalSize = aInstanceSize * distanceScale;
    
    // ========== 粒子特效参数 ==========
    float pulse = 1.0;
    vGlowIntensity = 0.0;
    vSpecularIntensity = 0.5;  // 默认镜面反射强度
    
    if (aInstanceType == 0) {      // 质子 - 光滑球体
        pulse = 1.0 + 0.06 * sin(uTime * 3.5 + aInstancePos.x * 10.0);
        vGlowIntensity = 0.2;
        vSpecularIntensity = 0.6;
    } 
    else if (aInstanceType == 1) { // 中子 - 粗糙球体
        pulse = 1.0 + 0.04 * sin(uTime * 2.5);
        vGlowIntensity = 0.2;
        vSpecularIntensity = 0.4;
    }
    else if (aInstanceType == 4) { // 光子 - 强发光，脉动
        pulse = 1.0 + 0.3 * sin(uTime * 15.0);
        vGlowIntensity = 1.2;
        vSpecularIntensity = 0.0;
        // 光子颜色随时间变化
        vColor = aInstanceColor;
        vColor.rgb = vec3(1.0, 0.9 + 0.1 * sin(uTime * 20.0), 0.5);
    }
    else if (aInstanceType == 5) { // 胶子 - 强发光，紫色光晕
        pulse = 1.0 + 0.15 * sin(uTime * 8.0);
        vGlowIntensity = 0.9;
        vSpecularIntensity = 0.1;
        vColor = aInstanceColor;
        vColor.rgb = vec3(0.75, 0.45, 0.95);
    }
    else if (aInstanceType == 6) { // W玻色子 - 红色发光
        pulse = 1.0 + 0.12 * sin(uTime * 6.0);
        vGlowIntensity = 0.8;
        vSpecularIntensity = 0.2;
        vColor = aInstanceColor;
        vColor.rgb = vec3(0.92, 0.28, 0.28);
    }
    else if (aInstanceType == 7) { // 上夸克 - 绿色发光
        pulse = 1.0 + 0.1 * sin(uTime * 7.0);
        vGlowIntensity = 0.6;
        vSpecularIntensity = 0.4;
        vColor = aInstanceColor;
        vColor.rgb = vec3(0.25, 0.88, 0.42);
    }
    else if (aInstanceType == 8) { // 下夸克 - 橙色发光
        pulse = 1.0 + 0.1 * sin(uTime * 7.5);
        vGlowIntensity = 0.6;
        vSpecularIntensity = 0.4;
        vColor = aInstanceColor;
        vColor.rgb = vec3(0.88, 0.55, 0.25);
    }
    
    finalSize *= pulse;
    
    vec3 finalPos = aPos * finalSize;
    worldPos = vec4(aInstancePos + finalPos, 1.0);
    gl_Position = uProjection * uView * worldPos;
    
    // 基础颜色
    if (aInstanceType != 4 && aInstanceType != 5 && aInstanceType != 6 && aInstanceType != 7 && aInstanceType != 8) {
        vColor = aInstanceColor;
        if (aInstanceType == 0) {
            vColor.rgb = vec3(0.95, 0.35, 0.25);
        } else if (aInstanceType == 1) {
            vColor.rgb = vec3(0.65, 0.68, 0.78);
        }
    }
}