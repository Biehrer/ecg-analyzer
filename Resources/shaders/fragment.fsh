#version 330

in vec3 fragmentColor;
out vec3 color;

uniform vec3 u_Color;

void main()
{

color = u_Color;

}

