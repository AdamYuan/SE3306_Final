#include "MeshLoader.hpp"

#include "Tumbler.hpp"

#include <cfloat>
#include <unordered_set>

void MeshLoader::load_obj_triangles(const char *filename) {
	m_triangles.clear();

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::material_t> materials;
	std::vector<tinyobj::shape_t> shapes;

	std::string load_warnings, load_errors;
	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &load_warnings, &load_errors, filename)) {
		printf("Failed to load %s\n", filename);
		return;
	}
	if (!load_errors.empty()) {
		printf("%s\n", load_errors.c_str());
		return;
	}
	if (!load_warnings.empty())
		printf("%s\n", load_warnings.c_str());

	if (materials.size() > 1)
		printf("too much materials\n");

	glm::vec3 pmin(FLT_MAX), pmax(-FLT_MAX);
	// Loop over shapes
	for (const auto &shape : shapes) {
		size_t index_offset = 0, face = 0;
		// Loop over faces(polygon)
		for (const auto &num_face_vertex : shape.mesh.num_face_vertices) {
			// Loop over triangles in the face.
			if (num_face_vertex != 3) {
				printf("Non-triangle face\n");
				continue;
			}
			Triangle t;
			for (size_t i = 0; i < num_face_vertex; ++i) {
				tinyobj::index_t index = shape.mesh.indices[index_offset + i];
				t[i] = {attrib.vertices[3 * index.vertex_index + 0], attrib.vertices[3 * index.vertex_index + 1],
				        attrib.vertices[3 * index.vertex_index + 2]};
			}

			m_triangles.push_back(t);

			index_offset += num_face_vertex;
			face++;
		}
	}
}

void MeshLoader::make_vertex_info_map() {
	m_vertex_map.clear();
	for (const auto &t : m_triangles) {
		glm::vec3 norm = glm::cross(t[1] - t[0], t[2] - t[0]);
		for (const auto &v : t) {
			auto it = m_vertex_map.find(v);
			if (it == m_vertex_map.end()) {
				uint32_t idx = m_vertex_map.size();
				it = m_vertex_map.insert({v, VertexInfo{idx, {}}}).first;
			}
			it->second.norm += norm;
		}
	}
}

Mesh MeshLoader::generate_mesh(const glm::vec3 &color) const {
	Mesh mesh = {};

	{ // push vertices to mesh
		mesh.m_vertices.resize(m_vertex_map.size());

		for (const auto &v : m_vertex_map) {
			auto &vert = mesh.m_vertices[v.second.idx];
			vert.position = v.first;
			vert.normal = glm::normalize(v.second.norm);
			vert.color = color;
		}
	}

	// push triangles
	for (const auto &tri : m_triangles) {
		mesh.m_triangles.push_back(
		    {m_vertex_map.at(tri[0]).idx, m_vertex_map.at(tri[1]).idx, m_vertex_map.at(tri[2]).idx});
	}
	return mesh;
}

Mesh MeshLoader::Load(const char *filename, const glm::vec3 &color) {
	load_obj_triangles(filename);
	make_vertex_info_map();
	return generate_mesh(color);
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

Mesh MeshLoader::MakeSphere(float radius, uint32_t subdivisions, const glm::vec3 &color) {
	make_sphere_triangles(radius, subdivisions);
	make_vertex_info_map();
	return generate_mesh(color);
}

Mesh MeshLoader::MakeCornellBox(const glm::vec3 &left_color, const glm::vec3 &right_color, const glm::vec3 &other_color,
                                const glm::vec3 &light_color, float light_height, float light_radius) {
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
	make_vertex_info_map();
	Mesh mesh = generate_mesh(other_color); // bottom

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
	make_vertex_info_map();
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
	make_vertex_info_map();
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
	make_vertex_info_map();
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
	make_vertex_info_map();
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
	make_vertex_info_map();
	mesh.Combine(generate_mesh(light_color)); // light

	return mesh;
}

Mesh MeshLoader::MakeTumbler(uint32_t y_subdivisions, uint32_t x_subdivisions, const glm::vec3 &color) {
	m_triangles.clear();

	float y_min = -Tumbler::kBottomRadius;
	std::vector<glm::vec2> y_r_vec;
	for (uint32_t i = 1; i <= y_subdivisions * 2; ++i) {
		float deg = float(i) / float(y_subdivisions * 2) * 2.0f * glm::pi<float>() / 3.0f;
		y_r_vec.emplace_back(-glm::cos(deg) * Tumbler::kBottomRadius, glm::sin(deg) * Tumbler::kBottomRadius);
	}

	float upper_sphere_y = 2.0f * (Tumbler::kBottomRadius - Tumbler::kTopRadius);
	float y_max = upper_sphere_y + Tumbler::kTopRadius;
	for (uint32_t i = y_subdivisions; i; --i) {
		float deg = float(i) / float(y_subdivisions) * glm::pi<float>() / 3.0f;
		y_r_vec.emplace_back(upper_sphere_y + glm::cos(deg) * Tumbler::kTopRadius, glm::sin(deg) * Tumbler::kTopRadius);
	}

	std::vector<float> deg_vec;
	for (uint32_t i = 0; i < x_subdivisions; ++i) {
		float deg = float(i) / float(x_subdivisions) * glm::pi<float>() * 2.0f;
		deg_vec.push_back(deg);
	}

	for (uint32_t i = 0; i < x_subdivisions; ++i) {
		float deg = deg_vec[i], next_deg = deg_vec[(i + 1) % x_subdivisions];
		m_triangles.push_back({
		    glm::vec3(0, y_min, 0),
		    glm::vec3(y_r_vec.front()[1] * glm::cos(deg), y_r_vec.front()[0], y_r_vec.front()[1] * glm::sin(deg)),
		    glm::vec3(y_r_vec.front()[1] * glm::cos(next_deg), y_r_vec.front()[0],
		              y_r_vec.front()[1] * glm::sin(next_deg)),
		});

		for (uint32_t j = 0; j < y_r_vec.size() - 1; ++j) {
			auto y_r = y_r_vec[j], next_y_r = y_r_vec[j + 1];
			glm::vec3 v0 = glm::vec3(y_r[1] * glm::cos(deg), y_r[0], y_r[1] * glm::sin(deg));
			glm::vec3 v1 = glm::vec3(y_r[1] * glm::cos(next_deg), y_r[0], y_r[1] * glm::sin(next_deg));
			glm::vec3 v2 = glm::vec3(next_y_r[1] * glm::cos(deg), next_y_r[0], next_y_r[1] * glm::sin(deg));
			glm::vec3 v3 = glm::vec3(next_y_r[1] * glm::cos(next_deg), next_y_r[0], next_y_r[1] * glm::sin(next_deg));
			m_triangles.push_back({v0, v3, v1});
			m_triangles.push_back({v0, v2, v3});
		}

		m_triangles.push_back({
		    glm::vec3(0, y_max, 0),
		    glm::vec3(y_r_vec.back()[1] * glm::cos(next_deg), y_r_vec.back()[0],
		              y_r_vec.back()[1] * glm::sin(next_deg)),
		    glm::vec3(y_r_vec.back()[1] * glm::cos(deg), y_r_vec.back()[0], y_r_vec.back()[1] * glm::sin(deg)),
		});
	}

	make_vertex_info_map();
	return generate_mesh(color);
}
