#version 130

in vec3 position;
in vec3 vertexColor;

out vec3 fragmentColor;

uniform mat4 u_MVP;
uniform float point_scale;

void main()
{

gl_Position = u_MVP * vec4(position, 1);

fragmentColor = vertexColor;

}


