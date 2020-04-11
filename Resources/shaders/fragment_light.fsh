#version 130

in vec3 fragmentColor;
out vec3 color;

uniform vec3 u_object_color;
uniform vec3 u_light_color;

void main()
{

color = u_light_color * u_object_color;

}

