#version 330 core

in vec4 fColor;
in float fAlpha;

uniform float uTime;

out vec4 FragColor;

void main() {
    // 圆形渐变（让点变成光晕）
    vec2 coord = gl_PointCoord * 2.0 - 1.0;
    float dist = length(coord);
    
    // 径向渐变：中心亮，边缘透明
    float radial = 1.0 - smoothstep(0.0, 0.7, dist);
    radial = pow(radial, 1.3);
    
    vec3 color = fColor.rgb;
    float alpha = fAlpha * radial;
    
    // 动态脉冲
    float pulse = 0.8 + 0.2 * sin(uTime * 10.0);
    color *= pulse;
    
    FragColor = vec4(color, alpha);
}