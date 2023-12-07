#include "MeshLoader.hpp"

#include "Sphere.hpp"
#include "Tumbler.hpp"

#include <unordered_set>

template <typename ColorFunc> Mesh MeshLoader::generate_mesh(ColorFunc &&color_func) const {
	std::unordered_map<glm::vec3, glm::vec3> vertex_normal_map;
	for (const auto &tri : m_triangles) {
		glm::vec3 norm = glm::cross(tri[1] - tri[0], tri[2] - tri[0]);
		for (const auto &vert : tri) {
			auto it = vertex_normal_map.find(vert);
			if (it == vertex_normal_map.end())
				it = vertex_normal_map.insert({vert, {}}).first;
			it->second += norm;
		}
	}
	for (auto &it : vertex_normal_map)
		it.second = glm::normalize(it.second);

	Mesh mesh = {};

	std::map<VertexKey, uint32_t> vertex_index_map;
	for (const auto &tri : m_triangles) {
		for (const auto &vert : tri) {
			if (vertex_index_map.count(vert))
				continue;
			uint32_t idx = mesh.m_vertices.size();
			mesh.m_vertices.push_back({
			    .position = vert,
			    .normal = vertex_normal_map[vert],
			    .color = color_func(vert),
			});
			vertex_index_map[vert] = idx;
		}
	}

	for (const auto &tri : m_triangles)
		mesh.m_triangles.push_back({vertex_index_map[tri[0]], vertex_index_map[tri[1]], vertex_index_map[tri[2]]});

	return mesh;
}

Mesh MeshLoader::generate_mesh(const glm::vec3 &color) const {
	return generate_mesh([&color](auto &&) { return color; });
}

void MeshLoader::make_sphere_triangles(float radius, uint32_t subdivisions) {
	constexpr static float kGoldenRatio = 1.61803398875f;
	constexpr static glm::vec3 kIcosahedronVertices[12] = {
	    glm::vec3(-1.0f, kGoldenRatio, 0.0f),  glm::vec3(1.0f, kGoldenRatio, 0.0f),
	    glm::vec3(-1.0f, -kGoldenRatio, 0.0f), glm::vec3(1.0f, -kGoldenRatio, 0.0f),
	    glm::vec3(0.0f, -1.0f, kGoldenRatio),  glm::vec3(0.0f, 1.0f, kGoldenRatio),
	    glm::vec3(0.0f, -1.0, -kGoldenRatio),  glm::vec3(0.0f, 1.0f, -kGoldenRatio),
	    glm::vec3(kGoldenRatio, 0.0f, -1.0f),  glm::vec3(kGoldenRatio, 0.0f, 1.0f),
	    glm::vec3(-kGoldenRatio, 0.0f, -1.0f), glm::vec3(-kGoldenRatio, 0.0, 1.0f),
	};
	constexpr static uint32_t kIcosahedronIndices[60] = {
	    0, 11, 5, 0, 5, 1, 0, 1, 7, 0, 7, 10, 0, 10, 11, 1, 5, 9, 5, 11, 4,  11, 10, 2,  10, 7, 6, 7, 1, 8,
	    3, 9,  4, 3, 4, 2, 3, 2, 6, 3, 6, 8,  3, 8,  9,  4, 9, 5, 2, 4,  11, 6,  2,  10, 8,  6, 7, 9, 8, 1,
	};

	m_triangles.clear();

	for (uint32_t i = 0; i < std::size(kIcosahedronIndices); i += 3) {
		m_triangles.push_back({
		    glm::normalize(kIcosahedronVertices[kIcosahedronIndices[i]]) * radius,
		    glm::normalize(kIcosahedronVertices[kIcosahedronIndices[i + 1]]) * radius,
		    glm::normalize(kIcosahedronVertices[kIcosahedronIndices[i + 2]]) * radius,
		});
	}
	const auto find_mid = [radius](const glm::vec3 &v0, const glm::vec3 &v1) {
		return glm::normalize(glm::vec3((v0.x + v1.x) * .5f, (v0.y + v1.y) * .5f, (v0.z + v1.z) * .5f)) * radius;
	};

	for (uint32_t i = 0; i < subdivisions; ++i) {
		std::vector<Triangle> new_triangles;
		for (const auto &tri : m_triangles) {
			auto mid01 = find_mid(tri[0], tri[1]);
			auto mid12 = find_mid(tri[1], tri[2]);
			auto mid02 = find_mid(tri[0], tri[2]);

			new_triangles.push_back({tri[0], mid01, mid02});
			new_triangles.push_back({mid01, tri[1], mid12});
			new_triangles.push_back({mid02, mid12, tri[2]});
			new_triangles.push_back({mid01, mid12, mid02});
		}
		m_triangles = std::move(new_triangles);
	}
}

void MeshLoader::make_revolution_triangles(float y_min, float y_max, std::span<const glm::vec2> y_r_s,
                                           uint32_t subdivisions, uint32_t edge_layer) {
	m_triangles.clear();

	std::vector<float> degs(subdivisions);
	for (uint32_t i = 0; i < subdivisions; ++i)
		degs[i] = float(i) / float(subdivisions) * glm::pi<float>() * 2.0f + glm::pi<float>();

	const auto get_deg_verts = [&](uint32_t j, uint32_t layer) {
		std::vector<VertexKey> verts;
		verts.reserve(y_r_s.size());
		for (auto y_r : y_r_s) {
			float deg = degs[j];
			verts.emplace_back(glm::vec3(y_r[1] * glm::cos(deg), y_r[0], y_r[1] * glm::sin(deg)), layer);
		}
		return verts;
	};

	const auto push_triangles = [&](std::span<const VertexKey> side_0, std::span<const VertexKey> side_1) {
		m_triangles.push_back({glm::vec3(.0f, y_min, .0f), side_0.front(), side_1.front()});
		for (uint32_t j = 0; j < y_r_s.size() - 1; ++j) {
			auto y_r = y_r_s[j], next_y_r = y_r_s[j + 1];
			const auto &v0 = side_0[j];
			const auto &v1 = side_1[j];
			const auto &v2 = side_0[j + 1];
			const auto &v3 = side_1[j + 1];
			m_triangles.push_back({v0, v3, v1});
			m_triangles.push_back({v0, v2, v3});
		}
		m_triangles.push_back({glm::vec3(.0f, y_max, .0f), side_1.back(), side_0.back()});
	};

	auto prev_side_verts = get_deg_verts(0, 0);
	for (uint32_t i = 0; i < subdivisions; ++i) {
		auto side_verts = get_deg_verts((i + 1) % subdivisions, i + 1 == subdivisions ? edge_layer : 0);
		push_triangles(prev_side_verts, side_verts);
		prev_side_verts = std::move(side_verts);
	}
}

Mesh MeshLoader::MakeIcoSphere(float radius, uint32_t subdivisions, const glm::vec3 &color) {
	make_sphere_triangles(radius, subdivisions);
	return generate_mesh(color);
}

Mesh MeshLoader::MakeUVSphere(float radius, uint32_t subdivisions, uint32_t texture) {
	auto xz_subdivisions = uint32_t((float)subdivisions * glm::pi<float>());

	std::vector<glm::vec2> y_r_vec;
	for (uint32_t i = 1; i < subdivisions; ++i) {
		float deg = float(i) / float(subdivisions) * glm::pi<float>();
		y_r_vec.emplace_back(-glm::cos(deg) * radius, glm::sin(deg) * radius);
	}

	make_revolution_triangles(-radius, radius, y_r_vec, xz_subdivisions, 1);
	return generate_mesh([&](VertexKey p) {
		p /= radius;
		return glm::vec3{-float(texture), p.layer == 0 ? glm::atan(p.z, p.x) / glm::pi<float>() : 1.0f,
		                 glm::asin(p.y) / glm::pi<float>()};
	});
}

Mesh MeshLoader::MakeCornellBox(const glm::vec3 &left_color, const glm::vec3 &right_color, uint32_t floor_texture,
                                const glm::vec3 &other_color, const glm::vec3 &light_color, float light_height,
                                float light_radius) {
	m_triangles = {{
	                   glm::vec3{-1.f, -1.f, -1.f},
	                   glm::vec3{+1.f, -1.f, +1.f},
	                   glm::vec3{+1.f, -1.f, -1.f},
	               },
	               {
	                   glm::vec3{-1.f, -1.f, -1.f},
	                   glm::vec3{-1.f, -1.f, +1.f},
	                   glm::vec3{+1.f, -1.f, +1.f},
	               }};
	Mesh mesh = generate_mesh(
	    [floor_texture](const glm::vec3 &p) { return glm::vec3(-float(floor_texture), p.x, p.z); }); // bottom

	m_triangles = {{
	                   glm::vec3{-1.f, 1.f, -1.f},
	                   glm::vec3{+1.f, 1.f, -1.f},
	                   glm::vec3{+1.f, 1.f, +1.f},
	               },
	               {
	                   glm::vec3{-1.f, 1.f, -1.f},
	                   glm::vec3{+1.f, 1.f, +1.f},
	                   glm::vec3{-1.f, 1.f, +1.f},
	               }};
	mesh.Combine(generate_mesh(other_color)); // top

	m_triangles = {{
	                   glm::vec3{-1.f, -1.f, -1.f},
	                   glm::vec3{+1.f, -1.f, -1.f},
	                   glm::vec3{+1.f, +1.f, -1.f},
	               },
	               {
	                   glm::vec3{-1.f, -1.f, -1.f},
	                   glm::vec3{+1.f, +1.f, -1.f},
	                   glm::vec3{-1.f, +1.f, -1.f},
	               }};
	mesh.Combine(generate_mesh(other_color)); // back

	m_triangles = {{
	                   glm::vec3{-1.f, -1.f, -1.f},
	                   glm::vec3{-1.f, +1.f, -1.f},
	                   glm::vec3{-1.f, +1.f, +1.f},
	               },
	               {
	                   glm::vec3{-1.f, -1.f, -1.f},
	                   glm::vec3{-1.f, +1.f, +1.f},
	                   glm::vec3{-1.f, -1.f, +1.f},
	               }};
	mesh.Combine(generate_mesh(left_color)); // left

	m_triangles = {{
	                   glm::vec3{1.f, -1.f, -1.f},
	                   glm::vec3{1.f, +1.f, +1.f},
	                   glm::vec3{1.f, +1.f, -1.f},
	               },
	               {
	                   glm::vec3{1.f, -1.f, -1.f},
	                   glm::vec3{1.f, -1.f, +1.f},
	                   glm::vec3{1.f, +1.f, +1.f},
	               }};
	mesh.Combine(generate_mesh(right_color)); // right

	make_sphere_triangles(light_radius, 4);
	for (auto &tri : m_triangles) {
		tri[0].y += light_height;
		tri[1].y += light_height;
		tri[2].y += light_height;
	}
	m_triangles.erase(
	    std::remove_if(m_triangles.begin(), m_triangles.end(),
	                   [](const auto &tri) { return tri[0].y > 1.f && tri[1].y > 1.f && tri[2].y > 1.f; }),
	    m_triangles.end());
	mesh.Combine(generate_mesh(light_color)); // light

	return mesh;
}

Mesh MeshLoader::MakeTumbler(uint32_t y_subdivisions, uint32_t xz_subdivisions, uint32_t texture) {
	float y_min = -Tumbler::kBottomRadius;
	std::vector<glm::vec2> y_r_vec;
	for (uint32_t i = 1; i <= y_subdivisions * 2; ++i) {
		float deg = float(i) / float(y_subdivisions * 2) * (glm::pi<float>() * 0.5f + Tumbler::kHalfAngle);
		y_r_vec.emplace_back(-glm::cos(deg) * Tumbler::kBottomRadius, glm::sin(deg) * Tumbler::kBottomRadius);
	}

	float y_max = Tumbler::kTopSphereY + Tumbler::kTopRadius;
	for (uint32_t i = y_subdivisions; i; --i) {
		float deg = float(i) / float(y_subdivisions) * (glm::pi<float>() * 0.5f - Tumbler::kHalfAngle);
		y_r_vec.emplace_back(Tumbler::kTopSphereY + glm::cos(deg) * Tumbler::kTopRadius,
		                     glm::sin(deg) * Tumbler::kTopRadius);
	}

	make_revolution_triangles(y_min, y_max, y_r_vec, xz_subdivisions, 1);
	return generate_mesh([&](VertexKey p) {
		return glm::vec3{-float(texture), p.layer == 0 ? glm::atan(p.z, p.x) / glm::pi<float>() : 1.0f,
		                 p.y / Tumbler::kTopSphereY};
	});
}
