#version 330 core

in vec4 vColor;
in float vAlpha;
in float vDistanceToCamera;
in float vParticleType;

uniform float uTime;

out vec4 FragColor;

void main() {
    vec3 color = vColor.rgb;
    float alpha = vAlpha;
    
    // 电子：添加光晕
    if (vParticleType == 2.0) {
        float glow = 0.5 + 0.3 * sin(uTime * 8.0);
        color += vec3(0.3, 0.6, 1.0) * glow;
        alpha = min(alpha + 0.2, 1.0);
    }
    
    // 光子：金色光晕
    if (vParticleType == 4.0) {
        color = vec3(1.0, 0.95, 0.6);
        alpha = 0.9;
    }
    
    // 胶子：紫色光晕
    if (vParticleType == 5.0) {
        color = vec3(0.7, 0.4, 0.9);
    }
    
    FragColor = vec4(color, alpha);
}