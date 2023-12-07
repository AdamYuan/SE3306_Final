#pragma once

#include <glm/glm.hpp>
#include <vector>

template <typename Derived> struct Particle {
	float life;
	glm::vec3 center, velocity;
};

struct FireParticle final : public Particle<FireParticle> {};
struct SparkParticle final : public Particle<SparkParticle> {};
struct AshParticle final : public Particle<AshParticle> {};

class ParticleSystem {
private:
	std::vector<FireParticle> m_fires;
	std::vector<SparkParticle> m_sparks;
	std::vector<AshParticle> m_ashes;

public:
};
