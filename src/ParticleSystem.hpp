#pragma once

#include <glm/glm.hpp>
#include <random>
#include <vector>

struct Particle {
	float life;
	glm::vec3 center, velocity;
};

struct FireParticle final : public Particle {
	inline glm::vec3 GetColor() const { return glm::vec3{}; }
	inline float GetRadius() const { return 0.01f; }
};
struct SparkParticle final : public Particle {
	inline glm::vec3 GetColor() const { return glm::vec3{}; }
	inline float GetRadius() const { return 0.01f; }
};
struct AshParticle final : public Particle {
	inline glm::vec3 GetColor() const { return glm::vec3{}; }
	inline float GetRadius() const { return 0.01f; }
};

class ParticleSystem {
private:
	std::vector<FireParticle> m_fires;
	std::vector<SparkParticle> m_sparks;
	std::vector<AshParticle> m_ashes;

	uint32_t m_max_particles;
	std::mt19937 m_rand{std::random_device{}()};

public:
	inline void Initialize(uint32_t max_particles) { m_max_particles = max_particles; }

};
