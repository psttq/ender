#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main()
{
  vec3 FragPos = vec3(aPos.x-1, aPos.y+1, aPos.z)/2;
  gl_Position = vec4(FragPos, 1.0);
	TexCoord = vec2(aTexCoord.x, aTexCoord.y);
}


