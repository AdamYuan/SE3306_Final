#version 450

#include "Util.h"

CONST_SPEC_MATRIX(0, kViewProj)

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;
layout(location = 3) in vec4 aInstanceColor;
layout(location = 4) in mat4 aModel;
layout(location = 8) in mat4 aPrevModel;

layout(location = 0) out vec3 vNormal;
layout(location = 1) out vec3 vColor;
layout(location = 2) out vec4 vClip;
layout(location = 3) out vec4 vPrevClip;

layout(push_constant) uniform uuPushConstant { vec2 uJitter; };

void main() {
	vNormal = mat3(aModel) * aNormal;
	vColor = mix(aColor, aInstanceColor.rgb, aInstanceColor.a);
	vPrevClip = kViewProj * aPrevModel * vec4(aPosition, 1.0);
	vClip = kViewProj * aModel * vec4(aPosition, 1.0);
	gl_Position = vec4(vClip.xy + uJitter * vClip.w, vClip.zw);
	gl_Position.y = -gl_Position.y;
}
