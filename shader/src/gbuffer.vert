#version 450

#include "Binding.h"

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;
layout(location = 3) in vec4 aInstanceColor;
layout(location = 4) in mat4 aModel;
layout(location = 8) in mat4 aPrevModel;

layout(location = 0) out vec3 vNormal;
layout(location = 1) out vec3 vColor;
layout(location = 2) out vec4 vPrevPos;

layout(location = 0) uniform vec2 uJitter;

layout(std140, binding = CAMERA_UNIFORM_BUFFER) uniform uuCamera {
	mat4 uViewProjection, uInverseViewProjection, uShadowViewProjection;
};

void main() {
	vNormal = mat3(aPrevModel) * aNormal;
	vColor = mix(aColor, aInstanceColor.rgb, aInstanceColor.a);
	vPrevPos = uViewProjection * aPrevModel * vec4(aPosition, 1.0);
	gl_Position = uViewProjection * aModel * vec4(aPosition, 1.0);
	gl_Position.xy += uJitter * gl_Position.w;
}
