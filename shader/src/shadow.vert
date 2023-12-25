#version 450

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;
layout(location = 3) in vec4 aInstanceColor;
layout(location = 4) in mat4 aModel;

layout(push_constant) uniform uuPushConstant { mat4 uShadowViewProj; };

void main() {
	gl_Position = uShadowViewProj * aModel * vec4(aPosition, 1.0);
	gl_Position.y = -gl_Position.y;
}
