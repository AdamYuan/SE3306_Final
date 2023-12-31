#include "ParticleSystem.hpp"

#include <algorithm>

constexpr float kFireParticleLife = .45f;
void FireParticle::UpdateVelocity(std::mt19937 *p_rand, float delta_t) {}
float FireParticle::GetRadius() const {
	return glm::log(life + 1.f) * .4f * glm::smoothstep(.0f, .02f, kFireParticleLife - life);
}
glm::vec3 FireParticle::GetColor() const {
	return glm::vec3{1.f, .4588f, .04f - life * 0.05f} * glm::max(life * 4.f + 1.f, 1.1f);
}
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

constexpr float kAshParticleLife = 1.f;
void AshParticle::UpdateVelocity(std::mt19937 *p_rand, float delta_t) {
	std::normal_distribution<float> speed_dis(.0f, 8.f);
	glm::vec3 acc = {speed_dis(*p_rand), speed_dis(*p_rand), speed_dis(*p_rand)};
	acc.y -= Marble::kGravity;
	this->velocity += acc * delta_t;
}
glm::vec3 AshParticle::GetColor() const {
	return glm::vec3{1.f, .4588f, .2f} * 5.f * glm::smoothstep(.01f, .015f, GetRadius());
}
float AshParticle::GetRadius() const { return Marble::kRadius * glm::min(glm::pow((life + 1.f) / 2.f, 7.f), 1.f); }
void ParticleSystem::EmitAshes(const Marble &marble) {
	std::uniform_int_distribution<uint32_t> count_dis{2u, 5u};
	uint32_t count = count_dis(m_rand);
	count = std::min(count, GetUnusedParticleCount());

	std::normal_distribution<float> life_dis{kAshParticleLife, 0.05f};
	while (count--) {
		AshParticle p = {};
		p.life = std::max(life_dis(m_rand), .0f);
		p.center = marble.center;
		p.velocity = marble.linear_velocity * .3f;
		m_ashes.push_back(p);
	}
}

inline static glm::mat3 normal_to_tbn(const glm::vec3 &normal) {
	glm::vec3 v1 = cross(normal, glm::vec3(0, 0, 1)), v2 = cross(normal, glm::vec3(0, 1, 0));
	glm::vec3 tangent = normalize(dot(v1, v1) > dot(v2, v2) ? v1 : v2);
	return {tangent, cross(tangent, normal), normal};
}

constexpr float kSparkParticleLife = 1.f;
void SparkParticle::UpdateVelocity(std::mt19937 *p_rand, float delta_t) {
	velocity.y -= Marble::kGravity * delta_t * .5f;
}
glm::vec3 SparkParticle::GetColor() const { return glm::vec3{1.f, .4588f, .1f} * (1.05f + life * 5.f); }
float SparkParticle::GetRadius() const { return 0.03f * life * life * life; }
void ParticleSystem::EmitSparks(const glm::vec3 &pos, const glm::vec3 &grad) {
	std::uniform_int_distribution<uint32_t> count_dis{16u, 24u};
	uint32_t count = count_dis(m_rand);
	count = std::min(count, GetUnusedParticleCount());
	glm::mat3 tbn = normal_to_tbn(grad);

	std::uniform_real_distribution<float> dir_dis{-1.f, 1.f};
	std::normal_distribution<float> grad_v_dis{.8f, .5f}, life_dis{kSparkParticleLife, 0.05f};
	while (count--) {
		SparkParticle p = {};
		p.life = std::max(life_dis(m_rand), .0f);
		glm::vec2 dir2;
		do {
			dir2 = {dir_dis(m_rand), dir_dis(m_rand)};
		} while (glm::dot(dir2, dir2) >= 1.f);
		dir2 *= glm::dot(dir2, dir2) * 2.5f;
		glm::vec3 dir = tbn * glm::vec3(dir2, grad_v_dis(m_rand));
		p.velocity = dir;
		p.center = pos + glm::normalize(dir) * Fireball::kRadius * .2f;
		m_sparks.push_back(p);
	}
}

void ParticleSystem::Update(float delta_t, GPUMesh *p_mesh) {
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

	pop_mesh_prev(p_mesh);
	const auto update = [&](auto &particles) {
		for (auto &p : particles)
			p.Update(&m_rand, delta_t);
	};
	update(m_fires);
	update(m_sparks);
	update(m_ashes);
	pop_mesh(p_mesh);
}

void ParticleSystem::pop_mesh_prev(GPUMesh *p_mesh) const {
	uint32_t count = 0;
	const auto pop_mesh_vec_prev = [&](auto &particles) {
		for (const auto &p : particles) {
			if (count >= m_max_particles)
				return;
			auto model = glm::identity<glm::mat4>();
			model[0][0] = model[1][1] = model[2][2] = p.GetRadius();
			model[3] = glm::vec4(p.center, 1.f);
			p_mesh->SetPrevModel(count, model);
			++count;
		}
	};
	pop_mesh_vec_prev(m_fires);
	pop_mesh_vec_prev(m_sparks);
	pop_mesh_vec_prev(m_ashes);
	p_mesh->SetInstanceCount(count);
}
void ParticleSystem::pop_mesh(GPUMesh *p_mesh) const {
	uint32_t count = 0;
	const auto pop_mesh_vec = [&](auto &particles) {
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
	pop_mesh_vec(m_fires);
	pop_mesh_vec(m_sparks);
	pop_mesh_vec(m_ashes);
}
