#version 450

#include "Binding.h"

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;
layout(location = 3) in mat4 aModel;

layout(std140, binding = CAMERA_UNIFORM_BUFFER) uniform uuCamera {
	mat4 uViewProjection, uInverseViewProjection, uShadowViewProjection;
};

void main() { gl_Position = uShadowViewProjection * aModel * vec4(aPosition, 1.0); }
