#version 130 
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 u_text_color;

void main()
{    
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    color = vec4(u_text_color, 1.0) * sampled;
}  