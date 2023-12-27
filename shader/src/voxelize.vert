#version 450

#include "Util.h"

CONST_SPEC_MATRIX(0, kShadowViewProj)

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;
layout(location = 3) in vec4 aInstanceColor;
layout(location = 4) in mat4 aModel;

layout(location = 0) out vec3 vNormal;
layout(location = 1) out vec3 vColor;
layout(location = 2) out vec4 vShadowPos;

void main() {
	vNormal = mat3(aModel) * aNormal;
	vColor = mix(aColor, aInstanceColor.rgb, aInstanceColor.a);
	gl_Position = aModel * vec4(aPosition, 1.0);
	vShadowPos = kShadowViewProj * gl_Position;
	vShadowPos.y = -vShadowPos.y;
}
