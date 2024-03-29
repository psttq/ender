#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 nearPoint;
out vec3 farPoint;

uniform mat4 view;
uniform mat4 projection;

vec3 UnprojectPoint(float x, float y, float z) {
    mat4 viewInv = inverse(view);
    mat4 projInv = inverse(projection);
    vec4 unprojectedPoint =  viewInv * projInv * vec4(x, y, z, 1.0);
    return unprojectedPoint.xyz / unprojectedPoint.w;
}

void main() {
    vec3 p = aPos.xyz;
    nearPoint = UnprojectPoint(p.x, p.y, 0.0).xyz; // unprojecting on the near plane
    farPoint = UnprojectPoint(p.x, p.y, 1.0).xyz; // unprojecting on the far plane
    gl_Position = vec4(p.x, p.y-0.01, p.z, 1.0); // using directly the clipped coordinates
}
