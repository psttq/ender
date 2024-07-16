#version 330

uniform int gObjectIndex;
uniform int gObjectParent;

out uvec3 FragColor;

void main()
{
   FragColor = uvec3(gObjectIndex, gObjectParent, gl_PrimitiveID);
}