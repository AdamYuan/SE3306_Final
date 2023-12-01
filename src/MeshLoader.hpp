#pragma once

#include <array>
#include <cfloat>
#include <cstring>
#include <glm/gtx/hash.hpp>
#include <tiny_obj_loader.h>
#include <unordered_map>

#include "Mesh.hpp"

class MeshLoader {
private:
	using Triangle = std::array<glm::vec3, 3>;
	struct VertexInfo {
		uint32_t idx;
		glm::vec3 norm;
	};
	std::vector<Triangle> m_triangles;
	std::unordered_map<glm::vec3, VertexInfo> m_vertex_map;

	void load_obj_triangles(const char *filename);
	void make_sphere_triangles(float radius, uint32_t subdivisions);
	void make_vertex_info_map();
	Mesh generate_mesh(const glm::vec3 &color) const;

public:
	Mesh Load(const char *filename, const glm::vec3 &color);
	Mesh MakeSphere(float radius, uint32_t subdivisions, const glm::vec3 &color);
	Mesh MakeCornellBox(const glm::vec3 &left_color, const glm::vec3 &right_color, const glm::vec3 &other_color,
	                    const glm::vec3 &light_color, float light_height, float light_radius);
};
