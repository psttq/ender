#version 330

uniform int gObjectIndex;
uniform int gDrawIndex;

out vec3 FragColor;

void main()
{
   FragColor = vec3(gObjectIndex, gDrawIndex, gl_PrimitiveID);
}