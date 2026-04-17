#version 330 core

in vec4 fColor;
in float fAlpha;
in vec2 fTexCoord;
in float fGlow;

out vec4 FragColor;

void main() {
    float dist = length(fTexCoord);
    float radialGradient = 1.0 - smoothstep(0.0, 1.0, dist);
    float alpha = fAlpha * radialGradient;
    
    vec3 color = fColor.rgb;
    color += vec3(0.3, 0.5, 0.8) * fGlow * radialGradient;
    
    FragColor = vec4(color, alpha);
}