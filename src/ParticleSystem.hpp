#pragma once

#include <glm/glm.hpp>
#include <random>
#include <vector>

#include "GPUMesh.hpp"
#include "Sphere.hpp"

template <typename Derived> struct Particle {
	float life;
	glm::vec3 center, velocity;
	inline void Update(std::mt19937 *p_rand, float delta_t) {
		static_cast<Derived *>(this)->UpdateVelocity(p_rand, delta_t);
		center += velocity * delta_t;
	}
};

struct FireParticle final : public Particle<FireParticle> {
	glm::vec3 GetColor() const;
	float GetRadius() const;
	void UpdateVelocity(std::mt19937 *p_rand, float delta_t);
};
struct SparkParticle final : public Particle<SparkParticle> {
	glm::vec3 GetColor() const;
	float GetRadius() const;
	void UpdateVelocity(std::mt19937 *p_rand, float delta_t);
};
struct AshParticle final : public Particle<AshParticle> {
	glm::vec3 GetColor() const;
	float GetRadius() const;
	void UpdateVelocity(std::mt19937 *p_rand, float delta_t);
};

class ParticleSystem {
private:
	std::vector<FireParticle> m_fires;
	std::vector<SparkParticle> m_sparks;
	std::vector<AshParticle> m_ashes;

	uint32_t m_max_particles;
	std::mt19937 m_rand{std::random_device{}()};

	float m_unused_fire_delta_t = 0.f;

	void pop_mesh(GPUMesh *p_mesh) const;

public:
	inline void Initialize(uint32_t max_particles) { m_max_particles = max_particles; }

	inline uint32_t GetParticleCount() const { return m_fires.size() + m_sparks.size() + m_ashes.size(); }
	inline uint32_t GetMaxParticleCount() const { return m_max_particles; }
	inline uint32_t GetUnusedParticleCount() const {
		return m_max_particles - std::min(GetParticleCount(), m_max_particles);
	}

	void SustainFire(const Fireball &fireball, float delta_t);
	void EmitAshes(const Marble &marble);
	void EmitSparks(const glm::vec3 &pos, const glm::vec3 &grad);

	void Update(float delta_t, GPUMesh *p_mesh);
};
