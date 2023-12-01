#pragma once

#include "Vertex.hpp"
#include <array>
#include <concepts>
#include <glm/gtx/quaternion.hpp>
#include <map>
#include <vector>

struct MeshTransform {
	glm::vec3 origin, aabb_min, aabb_max;
	glm::quat rotate;
	float scale;
};

class Mesh {
public:
	using Triangle = std::array<uint32_t, 3>;

private:
	std::vector<Vertex> m_vertices;
	std::vector<Triangle> m_triangles;

	friend class MeshLoader;
	friend class MeshSplitSerializer;
	template <typename> friend class Serializer;

public:
	[[nodiscard]] inline const auto &GetVertices() const { return m_vertices; }
	[[nodiscard]] inline const auto &GetTriangles() const { return m_triangles; }

	MeshTransform Normalize(bool y_zero);
	void Combine(const Mesh &r);
};
