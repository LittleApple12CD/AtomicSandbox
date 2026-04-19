#version 330 core

layout(points) in;
layout(points, max_vertices = 100) out;

uniform mat4 uProjection;
uniform mat4 uView;
uniform float uTime;

in vec4 vColor[];
in float vAlpha[];
in float vDistanceToCamera[];
in float vParticleType[];
in vec3 vWorldPos[];      // 电子的世界坐标

out vec4 fColor;
out float fAlpha;

// 简单的随机函数
float random(float seed) {
    return fract(sin(seed) * 43758.5453);
}

void main() {
    // 只为电子生成点云
    if (vParticleType[0] != 2.0) {
        gl_PointSize = 8.0;
        gl_Position = gl_in[0].gl_Position;
        fColor = vColor[0];
        fAlpha = vAlpha[0];
        EmitVertex();
        EndPrimitive();
        return;
    }
    
    // 获取电子的世界坐标（不是裁剪坐标！）
    vec3 electronWorldPos = vWorldPos[0];
    
    // 云的大小
    float cloudSize = 0.35;
    float dist = vDistanceToCamera[0];
    if (dist > 2.0) {
        cloudSize = 0.35 * (2.0 / dist);
    }
    cloudSize = max(cloudSize, 0.18);
    
    // 脉动
    float pulse = 0.7 + 0.3 * sin(uTime * 7.0);
    cloudSize *= pulse;
    
    // 生成60个点
    int numPoints = 60;
    
    for (int i = 0; i < numPoints; i++) {
        float seed1 = float(i) * 1.234;
        float seed2 = float(i) * 2.345 + uTime;
        float seed3 = float(i) * 3.456;
        
        // 球体内的随机点（在世界坐标系中生成）
        float theta = random(seed1) * 2.0 * 3.14159;
        float phi = acos(2.0 * random(seed2) - 1.0);
        float r = cloudSize * (0.3 + 0.7 * random(seed3));
        
        vec3 localPos = vec3(
            r * sin(phi) * cos(theta),
            r * sin(phi) * sin(theta),
            r * cos(phi)
        );
        
        // 云点的世界坐标 = 电子的世界坐标 + 局部偏移
        vec3 cloudWorldPos = electronWorldPos + localPos;
        
        // 正确转换到裁剪坐标
        vec4 viewPos = uView * vec4(cloudWorldPos, 1.0);
        gl_Position = uProjection * viewPos;
        
        // 半径比例
        float radiusRatio = r / cloudSize;
        
        // 点大小：中心大，边缘小
        float pointSize = 6.0 * (1.0 - radiusRatio * 0.5);
        gl_PointSize = max(pointSize, 2.0);
        
        // 颜色：中心白蓝，边缘淡蓝
        vec3 pointColor = mix(
            vec3(0.25, 0.5, 0.9),
            vec3(0.6, 0.8, 1.0),
            1.0 - radiusRatio
        );
        
        // 动态闪烁
        float twinkle = 0.7 + 0.5 * sin(uTime * 12.0 + localPos.x * 30.0);
        pointColor *= (0.7 + 0.3 * twinkle);
        
        fColor = vec4(pointColor, 1.0);
        fAlpha = 0.6 * (1.0 - radiusRatio * 0.4);
        
        EmitVertex();
        EndPrimitive();
    }
    
    // 中心核心大光晕
    for (int i = 0; i < 8; i++) {
        float seed = float(i) * 5.678;
        float theta = random(seed) * 2.0 * 3.14159;
        float phi = acos(2.0 * random(seed + 10.0) - 1.0);
        float r = cloudSize * 0.15 * random(seed + 20.0);
        
        vec3 localPos = vec3(
            r * sin(phi) * cos(theta),
            r * sin(phi) * sin(theta),
            r * cos(phi)
        );
        
        vec3 cloudWorldPos = electronWorldPos + localPos;
        vec4 viewPos = uView * vec4(cloudWorldPos, 1.0);
        gl_Position = uProjection * viewPos;
        
        gl_PointSize = 12.0;
        fColor = vec4(0.7, 0.85, 1.0, 1.0);
        fAlpha = 0.9;
        
        EmitVertex();
        EndPrimitive();
    }
}