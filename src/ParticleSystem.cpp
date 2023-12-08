#include "ParticleSystem.hpp"

#include <algorithm>

constexpr float kFireParticleLife = .45f;
void FireParticle::UpdateVelocity(std::mt19937 *p_rand, float delta_t) {}
float FireParticle::GetRadius() const {
	return glm::log(life + 1.f) * .4f * glm::smoothstep(.0f, .02f, kFireParticleLife - life);
}
glm::vec3 FireParticle::GetColor() const { return glm::vec3{1.f, .4588f, .0f} * glm::max(life * 4.8f, 1.05f); }

void SparkParticle::UpdateVelocity(std::mt19937 *p_rand, float delta_t) {}

constexpr float kAshParticleLife = 1.f;
void AshParticle::UpdateVelocity(std::mt19937 *p_rand, float delta_t) {
	std::normal_distribution<float> speed_dis(.0f, 8.f);
	glm::vec3 delta_v = {speed_dis(*p_rand), speed_dis(*p_rand), speed_dis(*p_rand)};
	delta_v.y -= Marble::kGravity;
	delta_v *= delta_t;
	this->velocity += delta_v;
}
glm::vec3 AshParticle::GetColor() const {
	return glm::vec3{1.f, .4588f, .0f} * 2.f * glm::smoothstep(.005f, .01f, GetRadius());
}
float AshParticle::GetRadius() const { return Marble::kRadius * glm::pow((life + 1.f) / 2.f, 10.f); }

void ParticleSystem::SustainFire(const Fireball &fireball, float delta_t) {
	delta_t += m_unused_fire_delta_t;
	m_unused_fire_delta_t = 0.f;

	constexpr float kFireParticlePerSec = 400.0f;
	auto count = uint32_t(kFireParticlePerSec * delta_t);
	m_unused_fire_delta_t += delta_t - float(count) / kFireParticlePerSec;

	std::normal_distribution<float> y_speed_dis(.9f, .2f), xz_speed_dis(.0f, .04f);
	std::uniform_real_distribution<float> pos_dis(-1.f, 1.f);

	count = std::min(count, GetUnusedParticleCount());
	while (count--) {
		glm::vec3 bias = {};
		do {
			bias = {pos_dis(m_rand), pos_dis(m_rand), pos_dis(m_rand)};
		} while (glm::length(bias) >= 1.f);
		bias *= glm::dot(bias, bias);

		FireParticle p = {};
		p.life = kFireParticleLife;
		p.center = fireball.center + bias * Fireball::kRadius * .95f;
		glm::vec3 base_velocity = fireball.GetVelocity(p.center);
		p.velocity = base_velocity * .1f + glm::vec3{xz_speed_dis(m_rand), y_speed_dis(m_rand), xz_speed_dis(m_rand)};
		m_fires.push_back(p);
	}
}

void ParticleSystem::EmitAshes(const Marble &marble) {
	std::uniform_int_distribution<uint32_t> count_dis{2u, 5u};
	uint32_t count = count_dis(m_rand);
	count = std::min(count, GetUnusedParticleCount());
	while (count--) {
		AshParticle p = {};
		p.life = kAshParticleLife;
		p.center = marble.center;
		p.velocity = marble.linear_velocity * .3f;
		m_ashes.push_back(p);
	}
}

void ParticleSystem::Update(float delta_t) {
	const auto update_life = [&](auto &particles) {
		for (auto &p : particles)
			p.life -= delta_t;
		particles.erase(
		    std::remove_if(std::begin(particles), std::end(particles), [](auto &&p) { return p.life <= .0f; }),
		    std::end(particles));
	};
	update_life(m_fires);
	update_life(m_sparks);
	update_life(m_ashes);

	const auto update = [&](auto &particles) {
		for (auto &p : particles)
			p.Update(&m_rand, delta_t);
	};
	update(m_fires);
	update(m_sparks);
	update(m_ashes);
}

void ParticleSystem::PopMesh(GPUMesh *p_mesh) const {
	uint32_t count = 0;
	const auto pop_mesh = [&](auto &particles) {
		for (const auto &p : particles) {
			if (count >= m_max_particles)
				return;
			auto model = glm::identity<glm::mat4>();
			model[0][0] = model[1][1] = model[2][2] = p.GetRadius();
			model[3] = glm::vec4(p.center, 1.f);
			p_mesh->SetModel(count, model);
			p_mesh->SetColor(count, glm::vec4{p.GetColor(), 1.f});
			++count;
		}
	};
	pop_mesh(m_fires);
	pop_mesh(m_sparks);
	pop_mesh(m_ashes);
	p_mesh->SetMeshCount(count);
}
