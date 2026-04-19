#version 330 core

in vec4 vColor;
in float vAlpha;
in float vDistanceToCamera;
in float vParticleType;
in vec3 vViewPos;
in vec3 vNormal;
in vec3 vWorldPos;
in float vGlowIntensity;
in float vSpecularIntensity;

uniform float uTime;

out vec4 FragColor;

// 噪声函数
float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

void main() {
    // 电子直接丢弃
    if (vParticleType == 2.0) {
        discard;
    }
    
    vec3 color = vColor.rgb;
    float alpha = vAlpha;
    float type = vParticleType;
    
    // ========== 光照计算 ==========
    vec3 lightDir = normalize(vec3(1.5, 2.0, 1.0));
    vec3 viewDir = normalize(-vViewPos);
    float diffuse = max(0.3, dot(vNormal, lightDir));
    vec3 reflectDir = reflect(-lightDir, vNormal);
    float specular = pow(max(0.0, dot(viewDir, reflectDir)), 32.0) * vSpecularIntensity;
    float fresnel = pow(1.0 - abs(dot(vNormal, viewDir)), 2.0);
    
    // 基础光照
    color = color * (diffuse + fresnel * 0.3);
    color += vec3(0.9, 0.9, 1.0) * specular;
    
    // ========== 发光效果（根据粒子类型） ==========
    float glowPulse = 0.6 + 0.4 * sin(uTime * 5.0);
    
    if (type == 0.0) {      // 质子 - 微弱光晕
        float glow = 0.3 + 0.15 * sin(uTime * 4.0);
        color += vec3(1.0, 0.4, 0.2) * glow * fresnel;
        color += vec3(0.8, 0.3, 0.15) * vGlowIntensity;
    }
    else if (type == 1.0) { // 中子 - 极微弱
        float glow = 0.15 + 0.08 * sin(uTime * 3.0);
        color += vec3(0.5, 0.5, 0.6) * glow;
    }
    else if (type == 4.0) { // 光子 - 强烈闪烁发光
        float flicker = 0.5 + 0.5 * sin(uTime * 25.0);
        float flicker2 = 0.3 + 0.3 * sin(uTime * 40.0);
        color = vec3(1.0, 0.95, 0.5);
        color += vec3(1.0, 0.8, 0.3) * (flicker + flicker2);
        color += vec3(0.8, 0.6, 0.2) * vGlowIntensity;
        alpha = 0.98;
        // 添加星芒效果
        float star = abs(sin(vWorldPos.x * 50.0 + uTime * 30.0)) * 0.3;
        color += vec3(1.0, 0.9, 0.6) * star;
    }
    else if (type == 5.0) { // 胶子 - 紫色光晕
        float glow = 0.6 + 0.3 * sin(uTime * 7.0);
        color += vec3(0.5, 0.2, 0.8) * glow * fresnel;
        color += vec3(0.6, 0.3, 0.9) * vGlowIntensity;
        alpha = min(alpha + 0.15, 0.96);
    }
    else if (type == 6.0) { // W玻色子 - 红色发光
        float glow = 0.7 + 0.3 * sin(uTime * 6.0);
        color += vec3(0.8, 0.15, 0.15) * glow;
        color += vec3(0.9, 0.2, 0.2) * vGlowIntensity * fresnel;
        alpha = min(alpha + 0.1, 0.95);
    }
    else if (type == 7.0) { // 上夸克 - 绿色发光
        float glow = 0.5 + 0.25 * sin(uTime * 8.0);
        color += vec3(0.2, 0.7, 0.3) * glow;
        color += vec3(0.25, 0.8, 0.4) * vGlowIntensity;
    }
    else if (type == 8.0) { // 下夸克 - 橙色发光
        float glow = 0.5 + 0.25 * sin(uTime * 9.0);
        color += vec3(0.7, 0.4, 0.15) * glow;
        color += vec3(0.8, 0.5, 0.2) * vGlowIntensity;
    }
    
    // ========== 粒子表面细节 ==========
    // 光子不需要表面纹理
    if (type != 4.0) {
        vec2 noiseCoord = vec2(vWorldPos.x * 5.0, vWorldPos.y * 5.0) + uTime * 0.2;
        float surfaceNoise = random(noiseCoord) * 0.08;
        color += surfaceNoise;
    }
    
    // ========== 发光光晕（所有发光粒子） ==========
    if (type != 0.0 && type != 1.0 && type != 2.0) {
        float outerGlow = vGlowIntensity * (0.5 + 0.3 * sin(uTime * 8.0));
        color += vec3(0.5, 0.6, 0.9) * outerGlow * fresnel;
    }
    
    // ========== 距离雾化 ==========
    float fogFactor = clamp(vDistanceToCamera / 12.0, 0.0, 0.35);
    vec3 fogColor = vec3(0.05, 0.05, 0.12);
    color = mix(color, fogColor, fogFactor);
    
    // 限制颜色范围
    color = clamp(color, 0.0, 1.0);
    alpha = clamp(alpha, 0.0, 1.0);
    
    FragColor = vec4(color, alpha);
}