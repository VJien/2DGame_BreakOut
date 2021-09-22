#pragma once
#include "glHeader.h"
#include "vector"
#include "Texture.h"

class GameObject;

struct Particle
{
	glm::vec2 Position, Velocity;
	glm::vec4 Color;
	float     Life;
	Particle() : Position(0.0f), Velocity(0.0f), Color(1.0f), Life(0.0f) { }
};







class ParticleGenerator
{
public:
	// constructor
	ParticleGenerator(Shader shader, Texture2D texture, unsigned int amount);
	// update all particles
	void Update(float dt, GameObject& object, unsigned int newParticles, glm::vec2 offset = glm::vec2(0.0f, 0.0f));
	// render all particles
	void Draw();

private:
	// state
	std::vector<Particle> particles;
	unsigned int amount;

	Shader shader;
	Texture2D texture;

	unsigned int VAO;

	void init();
	// 返回第一个生命值结束的粒子序号
	unsigned int firstUnusedParticle();
	// 重新生成粒子
	void respawnParticle(Particle& particle, GameObject& object, glm::vec2 offset = glm::vec2(0.0f, 0.0f));

};

