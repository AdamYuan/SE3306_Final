#pragma once

#include <array>
#include <cfloat>
#include <cstring>
#include <glm/gtx/hash.hpp>
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

	void make_sphere_triangles(float radius, uint32_t subdivisions);
	void make_vertex_info_map();
	template <typename ColorFunc> Mesh generate_mesh(ColorFunc &&color_func) const;
	Mesh generate_mesh(const glm::vec3 &color) const;

public:
	Mesh MakeSphere(float radius, uint32_t subdivisions, const glm::vec3 &color);
	Mesh MakeCornellBox(const glm::vec3 &left_color, const glm::vec3 &right_color, uint32_t floor_texture,
	                    const glm::vec3 &other_color, const glm::vec3 &light_color, float light_height,
	                    float light_radius);
	Mesh MakeTumbler(uint32_t y_subdivisions, uint32_t x_subdivisions, const glm::vec3 &color);
};
