#version 450 core

#include "Config.h"

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

layout(location = 0) in vec3 vNormal[];
layout(location = 1) in vec3 vColor[];
layout(location = 2) in vec4 vShadowPos[];

layout(location = 0) out vec3 gNormal;
layout(location = 1) out vec3 gColor;

// for direct light calculation
layout(location = 2) out vec3 gWorldPos;
layout(location = 3) out vec4 gShadowPos;

vec2 Project(vec3 v, in const uint axis) { return (axis == 0 ? v.yz : (axis == 1 ? v.zx : v.xy)) * VOXEL_SCALE; }

void main() {
	vec3 pos0 = gl_in[0].gl_Position.xyz;
	vec3 pos1 = gl_in[1].gl_Position.xyz;
	vec3 pos2 = gl_in[2].gl_Position.xyz;

	vec3 axis_weight = abs(cross(pos1 - pos0, pos2 - pos0));
	uint axis = (axis_weight.x > axis_weight.y && axis_weight.x > axis_weight.z)
	                ? 0
	                : ((axis_weight.y > axis_weight.z) ? 1 : 2);

	gNormal = vNormal[0];
	gColor = vColor[0];
	gWorldPos = pos0;
	gShadowPos = vShadowPos[0];
	gl_Position = vec4(Project(pos0, axis), 0.0, 1.0);
	EmitVertex();
	gNormal = vNormal[1];
	gColor = vColor[1];
	gWorldPos = pos1;
	gShadowPos = vShadowPos[1];
	gl_Position = vec4(Project(pos1, axis), 0.0, 1.0);
	EmitVertex();
	gNormal = vNormal[2];
	gColor = vColor[2];
	gWorldPos = pos2;
	gShadowPos = vShadowPos[2];
	gl_Position = vec4(Project(pos2, axis), 0.0, 1.0);
	EmitVertex();
	EndPrimitive();
}
