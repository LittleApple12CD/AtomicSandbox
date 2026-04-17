#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aInstancePos;
layout(location = 2) in vec4 aInstanceColor;
layout(location = 3) in float aInstanceSize;
layout(location = 4) in int aInstanceType;

uniform mat4 uView;
uniform mat4 uProjection;
uniform float uTime;

out vec4 vColor;
out float vAlpha;
out float vDistanceToCamera;
out float vParticleType;

void main() {
    vec4 viewPos = uView * vec4(aInstancePos, 1.0);
    float distanceToCamera = length(viewPos.xyz);
    vDistanceToCamera = distanceToCamera;
    vParticleType = float(aInstanceType);
    
    // 透明度
    float alpha = 1.0;
    if (distanceToCamera > 5.0) {
        alpha = 5.0 / distanceToCamera;
    }
    vAlpha = clamp(alpha, 0.4, 1.0);
    
    // 大小
    float finalSize = aInstanceSize;
    if (distanceToCamera > 3.0) {
        finalSize = aInstanceSize * (3.0 / distanceToCamera);
    }
    finalSize = max(finalSize, 0.015);
    
    // 电子脉动
    if (aInstanceType == 2) {
        float pulse = 0.8 + 0.2 * sin(uTime * 10.0 + aInstancePos.x * 30.0);
        finalSize *= pulse;
    }
    
    vec3 scaledPos = aPos * finalSize;
    vec4 worldPos = vec4(aInstancePos + scaledPos, 1.0);
    gl_Position = uProjection * uView * worldPos;
    
    vColor = aInstanceColor;
    
    // 电子更亮
    if (aInstanceType == 2) {
        vColor.rgb = vColor.rgb * 1.5;
    }
}