#pragma once

#include "GPUMesh.hpp"
#include "Sphere.hpp"
#include "Tumbler.hpp"

#include <array>
#include <optional>
#include <vector>

class Playground {
private:
	std::vector<Tumbler> m_tumblers;
	std::vector<Sphere> m_spheres;

	std::optional<uint32_t> m_opt_lock;

public:
	struct LockInfo {
		float t;
		bool below_center;
	};
	void Initialize(uint32_t tumbler_count, float place_radius);
	void SetTumblerMesh(GPUMesh *p_mesh, uint32_t begin_id = 0) const;
	std::optional<LockInfo> TryLockTumbler(const glm::vec3 &origin, const glm::vec3 &dir);
	void MoveLockedTumbler(const glm::vec2 &offset);
	void RotateLockedTumbler(const glm::vec2 &offset, float rotate_angle);
	void UnlockTumbler();
	void Update(float delta_t);
};
