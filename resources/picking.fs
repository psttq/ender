#version 330

uniform int gObjectIndex;
uniform int gDrawIndex;

out uvec3 FragColor;

void main()
{
   FragColor = uvec3(gObjectIndex, gDrawIndex, gl_PrimitiveID);
}