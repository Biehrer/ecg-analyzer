#version 130 
in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat4 u_MVP;

void main()
{
    gl_Position = u_MVP * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}  