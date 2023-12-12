#pragma once

#include <array>
#include <cfloat>
#include <cstring>
#include <glm/gtx/hash.hpp>
#include <span>
#include <unordered_map>

#include "Mesh.hpp"

class MeshLoader {
private:
	struct VertexKey final : public glm::vec3 {
		uint32_t layer;
		inline VertexKey(const glm::vec3 &v, uint32_t layer = 0) : glm::vec3(v), layer{layer} {}
		inline bool operator<(const VertexKey &r) const {
			return std::tie(x, y, z, layer) < std::tie(r.x, r.y, r.z, r.layer);
		}
	};
	using Triangle = std::array<VertexKey, 3>;
	std::vector<Triangle> m_triangles;

	void make_sphere_triangles(float radius, uint32_t subdivisions);
	void make_revolution_triangles(float y_min, float y_max, std::span<const glm::vec2> y_r_s, uint32_t subdivisions,
	                               uint32_t edge_layer);
	template <typename ColorFunc> Mesh generate_mesh(ColorFunc &&color_func) const;
	Mesh generate_mesh(const glm::vec3 &color) const;

public:
	Mesh MakeIcoSphere(float radius, uint32_t subdivisions, const glm::vec3 &color);
	Mesh MakeUVSphere(float radius, uint32_t subdivisions, uint32_t texture);
	Mesh MakeCornellBox(const glm::vec3 &left_color, const glm::vec3 &right_color, uint32_t floor_texture,
	                    const glm::vec3 &other_color, const glm::vec3 &light_color, float light_height,
	                    float light_radius, uint32_t light_ico_subdivision);
	Mesh MakeTumbler(uint32_t y_subdivisions, uint32_t xz_subdivisions, uint32_t texture);
};
