#include "Mesh.hpp"

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
