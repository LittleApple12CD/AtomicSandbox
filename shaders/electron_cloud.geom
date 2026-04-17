#version 330 core

layout(points) in;
layout(triangle_strip, max_vertices = 16) out;

uniform mat4 uProjection;
uniform mat4 uView;
uniform float uTime;

in vec4 vColor[];
in float vAlpha[];
in float vDistanceToCamera[];
in float vParticleType[];

out vec4 fColor;
out float fAlpha;
out vec2 fTexCoord;
out float fGlow;

void main() {
    vec4 center = gl_in[0].gl_Position;
    
    float size = 0.15;
    float pulse = 0.8 + 0.4 * sin(uTime * 8.0);
    size *= pulse;
    
    vec3 right = vec3(uView[0][0], uView[1][0], uView[2][0]);
    vec3 up = vec3(uView[0][1], uView[1][1], uView[2][1]);
    
    vec3 corners[4] = vec3[](
        vec3(-size, -size, 0.0),
        vec3( size, -size, 0.0),
        vec3(-size,  size, 0.0),
        vec3( size,  size, 0.0)
    );
    
    for (int i = 0; i < 4; i++) {
        vec3 offset = right * corners[i].x + up * corners[i].y;
        gl_Position = uProjection * (center + vec4(offset, 0.0));
        fColor = vColor[0];
        fAlpha = 0.6;
        fTexCoord = vec2(corners[i].x / size, corners[i].y / size);
        fGlow = 0.5;
        EmitVertex();
    }
    EndPrimitive();
}