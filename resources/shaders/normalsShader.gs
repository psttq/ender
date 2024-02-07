#version 330

layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

vec3 GetNormal()
{
   vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
   vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
   return normalize(cross(a, b));
}

vec3    calcNormal ( int i0, int i1, int i2 )
{
    vec3    va = gl_in [i1].gl_Position.xyz - gl_in [i0].gl_Position.xyz;
    vec3    vb = gl_in [i2].gl_Position.xyz - gl_in [i0].gl_Position.xyz;

    return normalize ( cross ( va, vb ) );
}


void main( void )
{
    vec3 N = GetNormal();



    for(int i=0; i < gl_in.length(); ++i)
    {
        gl_Position = gl_in[i].gl_Position;
        Normal = N;
        EmitVertex();
    }

    EndPrimitive();
}
