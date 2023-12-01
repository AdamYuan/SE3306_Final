#version 450

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;
layout(location = 3) in mat4 aModel;

layout(location = 0) out vec3 vNormal;
layout(location = 1) out vec3 vColor;

layout(std140, binding = 0) uniform uuCamera { mat4 uViewProjection, uInverseViewProjection; };

void main() {
	vNormal = mat3(aModel) * aNormal;
	vColor = aColor;
	gl_Position = uViewProjection * aModel * vec4(aPosition, 1.0);
}
