#include "Mesh.hpp"

MeshTransform Mesh::Normalize(bool y_zero) {
	glm::vec3 vmin{FLT_MAX}, vmax{-FLT_MAX};

	// find vertex bound
	for (const auto &v : m_vertices) {
		vmin = glm::min(v.position, vmin);
		vmax = glm::max(v.position, vmax);
	}

	MeshTransform transform = {};

	{ // transform vertices
		glm::vec3 extent3 = vmax - vmin;
		float extent = glm::max(extent3.x, glm::max(extent3.y, extent3.z)) * 0.5f;
		float inv_extent = 1.0f / extent;
		glm::vec3 center = (vmax + vmin) * 0.5f;
		if (y_zero)
			center.y -= extent3.y * 0.5f;

		transform.origin = center;
		transform.scale = extent;
		transform.aabb_min = (vmin - center) * inv_extent;
		transform.aabb_max = (vmax - center) * inv_extent;

		for (auto &v : m_vertices)
			v.position = (v.position - center) * inv_extent;
	}

	return transform;
}

void Mesh::Combine(const Mesh &r) {
	uint32_t vert_base = m_vertices.size();
	m_vertices.insert(m_vertices.end(), r.m_vertices.begin(), r.m_vertices.end());
	uint32_t tri_base = m_triangles.size();
	m_triangles.insert(m_triangles.end(), r.m_triangles.begin(), r.m_triangles.end());
	for (std::size_t i = tri_base; i < m_triangles.size(); ++i) {
		m_triangles[i][0] += vert_base;
		m_triangles[i][1] += vert_base;
		m_triangles[i][2] += vert_base;
	}
}
