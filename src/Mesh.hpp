#pragma once

#include "Vertex.hpp"
#include <array>
#include <concepts>
#include <glm/gtx/quaternion.hpp>
#include <map>
#include <vector>

class Mesh {
public:
	using Triangle = std::array<uint32_t, 3>;

private:
	std::vector<Vertex> m_vertices;
	std::vector<Triangle> m_triangles;

	friend class MeshLoader;

public:
	[[nodiscard]] inline const auto &GetVertices() const { return m_vertices; }
	[[nodiscard]] inline const auto &GetTriangles() const { return m_triangles; }

	void Combine(const Mesh &r);
};
