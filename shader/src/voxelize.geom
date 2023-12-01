#version 450 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

layout(location = 0) in vec3 vNormal[];
layout(location = 1) in vec3 vColor[];

layout(location = 0) out vec3 gNormal;
layout(location = 1) out vec3 gColor;
layout(location = 2) flat out uint gAxis;

vec3 Project(in vec3 v, in uint axis) {
	vec3 ret = axis == 0 ? v.yzx : (axis == 1 ? v.zxy : v.xyz);
	return ret;
}

void main() {
	vec3 pos0 = gl_in[0].gl_Position.xyz;
	vec3 pos1 = gl_in[1].gl_Position.xyz;
	vec3 pos2 = gl_in[2].gl_Position.xyz;

	vec3 axis_weight = abs(cross(pos1 - pos0, pos2 - pos0));
	uint axis = (axis_weight.x > axis_weight.y && axis_weight.x > axis_weight.z)
	                ? 0
	                : ((axis_weight.y > axis_weight.z) ? 1 : 2);

	vec3 center = (pos0 + pos1 + pos2) * 0.33333333333;
	pos0 += normalize(pos0 - center) * 0.02;
	pos1 += normalize(pos1 - center) * 0.02;
	pos2 += normalize(pos2 - center) * 0.02;

	gAxis = axis;

	gNormal = vNormal[0];
	gColor = vColor[0];
	gl_Position = vec4(Project(pos0, axis), 1.0);
	EmitVertex();
	gNormal = vNormal[1];
	gColor = vColor[1];
	gl_Position = vec4(Project(pos1, axis), 1.0);
	EmitVertex();
	gNormal = vNormal[2];
	gColor = vColor[2];
	gl_Position = vec4(Project(pos2, axis), 1.0);
	EmitVertex();
	EndPrimitive();
}
