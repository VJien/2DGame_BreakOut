#include "game.h"
#include "Sprite2D.h"
#include "Resource.h"
#include "game_level.h"
#include "ball_object.h"
#include "game_object.h"
#include "Particle_Generator.h"
#include "post_processor.h"
#include "PowerUp.h"
#include "irrKlang.h"
#include "text_render.h"
#include "iosfwd"
#include <iostream>
#include "string"
#include<sstream> 
#include <algorithm>

using namespace irrklang;

Sprite2D* sprite; 
GameObject* Player;
BallObject* Ball;
ParticleGenerator* Particles;
PostProcessor* Effects;
ISoundEngine* SoundEngine = createIrrKlangDevice();
TextRenderer* Text;



float ShakeTime = 0.0f;

bool CheckCollision(GameObject& one, GameObject& two);
Collision CheckCollision(BallObject& one, GameObject& two);
Direction VectorDirection(glm::vec2 target);
void ActivatePowerUp(PowerUp& powerUp);

bool CheckCollision(GameObject& one, GameObject& two)
{
	bool bCollisionX = one.Position.x + one.Size.x >= two.Position.x && two.Position.x + two.Size.x >= one.Position.x;
	bool bCollisionY = one.Position.y + one.Size.y >= two.Position.y && two.Position.y + two.Size.y >= one.Position.y;
	return bCollisionX && bCollisionY;
}

Collision CheckCollision(BallObject& one, GameObject& two)
{
	glm::vec2 center(one.Position + one.Radius);
	//aabb的半长
	glm::vec2 aabb_half_extents(two.Size.x / 2.0f, two.Size.y / 2.0f);
	//aabb的中心位置
	glm::vec2 aabb_center(two.Position.x + aabb_half_extents.x, two.Position.y + aabb_half_extents.y);
	// 获取两个中心的差矢量
	glm::vec2 difference = center - aabb_center;

	glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
	// AABB_center加上clamped这样就得到了碰撞箱上距离圆最近的点closest
	glm::vec2 closest = aabb_center + clamped;

	difference = closest - center;

	if (glm::length(difference) < one.Radius) 
		return std::make_tuple(true, VectorDirection(difference), difference);
	else
		return std::make_tuple(false, UP, glm::vec2(0.0f, 0.0f));
}

Direction VectorDirection(glm::vec2 target)
{
	glm::vec2 compass[] = {
		glm::vec2(0.0f, 1.0f),	// up
		glm::vec2(1.0f, 0.0f),	// right
		glm::vec2(0.0f, -1.0f),	// down
		glm::vec2(-1.0f, 0.0f)	// left
	};
	float max = 0.0f;
	unsigned int best_match = -1;
	for (unsigned int i = 0; i < 4; i++)
	{
		float dot_product = glm::dot(glm::normalize(target), compass[i]);
		if (dot_product > max)
		{
			max = dot_product;
			best_match = i;
		}
	}
	return (Direction)best_match;
}


void Game::DoCollisions()
{
	for (GameObject& box : this->Levels[this->Level].Bricks)
	{
		if (!box.Destroyed)
		{
			Collision collision = CheckCollision(*Ball, box);
			if (std::get<0>(collision)) // 如果collision 是 true
			{
				// 如果砖块不是实心就销毁砖块
				if (!box.IsSolid)
				{
					box.Destroyed = GL_TRUE;
					SpawnPowerUps(box);
					SoundEngine->play2D("Resource/audio/bleep.mp3", false);
					this->Score += rand() % 10 ;
					this->MaxScore = std::max(MaxScore, Score);
				}
				else//碰到无法销毁的砖块
				{
					ShakeTime = 0.1f;
					Effects->Shake = true;
					SoundEngine->play2D("Resource/audio/solid.wav", false);
				}
				// 碰撞处理
				Direction dir = std::get<1>(collision);
				glm::vec2 diff_vector = std::get<2>(collision);

				if (dir == LEFT || dir == RIGHT) // 水平方向碰撞
				{
					Ball->Velocity.x = -Ball->Velocity.x; // 反转水平速度
					// 重定位
					GLfloat penetration = Ball->Radius - std::abs(diff_vector.x);
					if (dir == LEFT)
						Ball->Position.x += penetration; // 将球右移
					else
						Ball->Position.x -= penetration; // 将球左移
				}
				else // 垂直方向碰撞
				{
					Ball->Velocity.y = -Ball->Velocity.y; // 反转垂直速度
					// 重定位
					GLfloat penetration = Ball->Radius - std::abs(diff_vector.y);
					if (dir == UP)
						Ball->Position.y -= penetration; // 将球上移
					else
						Ball->Position.y += penetration; // 将球下移
				}
			}
		}
	}

	for (PowerUp& powerUp : this->PowerUps)
	{
		if (!powerUp.Destroyed)
		{
			// first check if powerup passed bottom edge, if so: keep as inactive and destroy
			if (powerUp.Position.y >= this->Height)
				powerUp.Destroyed = true;

			if (CheckCollision(*Player, powerUp))
			{	// collided with player, now activate powerup
				ActivatePowerUp(powerUp);
				powerUp.Destroyed = true;
				powerUp.Activated = true;
				SoundEngine->play2D("Resource/audio/powerup.wav", false);
				this->Score += rand() % 10;
				this->MaxScore = std::max(MaxScore, Score);
			}
		}
	}


	Collision result = CheckCollision(*Ball, *Player);
	if (!Ball->Stuck && std::get<0>(result))
	{
		// 检查碰到了挡板的哪个位置，并根据碰到哪个位置来改变速度
		GLfloat centerBoard = Player->Position.x + Player->Size.x / 2;
		GLfloat distance = (Ball->Position.x + Ball->Radius) - centerBoard;
		GLfloat percentage = distance / (Player->Size.x / 2);
		// 依据结果移动
		GLfloat strength = 2.0f;
		glm::vec2 oldVelocity = Ball->Velocity;
		Ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength;
		Ball->Velocity.y = -Ball->Velocity.y;
		Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity);

		Ball->Velocity.y = -1.0f * abs(Ball->Velocity.y);
		Ball->Stuck = Ball->Sticky;

		SoundEngine->play2D("Resource/audio/bleep.wav", false);


	}

}

Game::Game(GLuint width, GLuint height)
	: State(GAME_MENU), Keys(), Width(width), Height(height), Level(0), Lives(3), Score(0) ,  MaxScore(0)
{

}

Game::~Game()
{
	delete sprite;
	delete Player;
	delete Ball;
	delete Particles;
	delete Effects;
	SoundEngine->drop();
	delete Text;
}

void Game::Init()
{
	// 加载着色器
	Resource::LoadShader("Shaders/sprite.vert", "Shaders/sprite.frag", nullptr, "sprite");
	Resource::LoadShader("shaders/particle.vert", "shaders/particle.frag", nullptr, "particle"); //粒子着色器
	Resource::LoadShader("shaders/post_processing.vert", "shaders/post_processing.frag", nullptr, "postprocessing");


	glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(this->Width),
		static_cast<GLfloat>(this->Height), 0.0f, -1.0f, 1.0f);
	Resource::GetShader("sprite").Use().SetInteger("image", 0);
	Resource::GetShader("sprite").SetMatrix4("projection", projection);
	//粒子
	Resource::GetShader("particle").Use().SetInteger("sprite", 0);
	Resource::GetShader("particle").SetMatrix4("projection", projection);


	



	// 加载纹理
	Resource::LoadTexture("Resource/Textures/background.jpg", GL_FALSE, "background");
	Resource::LoadTexture("Resource/Textures/awesomeface.png", GL_TRUE, "face");
	Resource::LoadTexture("Resource/Textures/block.png", GL_FALSE, "block");
	Resource::LoadTexture("Resource/Textures/block_solid.png", GL_FALSE, "block_solid");
	Resource::LoadTexture("Resource/Textures/paddle.png", GL_TRUE, "paddle");
	Resource::LoadTexture("Resource/Textures/particle.png", true, "particle");//粒子贴图
	Resource::LoadTexture("Resource/Textures/powerup_speed.png", true, "powerup_speed");
	Resource::LoadTexture("Resource/Textures/powerup_sticky.png", true, "powerup_sticky");
	Resource::LoadTexture("Resource/Textures/powerup_increase.png", true, "powerup_increase");
	Resource::LoadTexture("Resource/Textures/powerup_confuse.png", true, "powerup_confuse");
	Resource::LoadTexture("Resource/Textures/powerup_chaos.png", true, "powerup_chaos");
	Resource::LoadTexture("Resource/Textures/powerup_passthrough.png", true, "powerup_passthrough");


	// 加载关卡
	GameLevel one; one.Load("Resource/levels/one.lvl", this->Width, this->Height * 0.5);
	GameLevel two; two.Load("Resource/levels/two.lvl", this->Width, this->Height * 0.5);
	GameLevel three; three.Load("Resource/levels/three.lvl", this->Width, this->Height * 0.5);
	GameLevel four; four.Load("Resource/levels/four.lvl", this->Width, this->Height * 0.5);
	this->Levels.push_back(one);
	this->Levels.push_back(two);
	this->Levels.push_back(three);
	this->Levels.push_back(four);
	this->Level = 1;

	glm::vec2 playerPos = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);
	Player = new GameObject(playerPos, PLAYER_SIZE, Resource::GetTexture("paddle"));

	glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2 - BALL_RADIUS, -BALL_RADIUS * 2);
	Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY,Resource::GetTexture("face"));


	// 设置专用于渲染的控制
	sprite = new Sprite2D(Resource::GetShader("sprite"));
	//粒子发射器
	Particles = new ParticleGenerator(Resource::GetShader("particle"), Resource::GetTexture("particle"), 500);
	//后期
	Effects = new PostProcessor(Resource::GetShader("postprocessing"), this->Width, this->Height);

	Text = new TextRenderer(this->Width, this->Height);
	//Text->Load("Resource/fonts/OCRAEXT.TTF", 24);
	Text->Load("Resource/fonts/Antonio-Light.TTF", 24);

	SoundEngine->play2D("Resource/audio/breakout.mp3", true);
}

void Game::ProcessInput(GLfloat dt)
{
	if (this->State == GAME_MENU)
	{
		if (this->Keys[GLFW_KEY_ENTER] && !this->KeysProcessed[GLFW_KEY_ENTER])
		{
			this->State = GAME_ACTIVE;
			this->KeysProcessed[GLFW_KEY_ENTER] = true;
		}
		if (this->Keys[GLFW_KEY_W] && !this->KeysProcessed[GLFW_KEY_W])
		{
			this->Level = (this->Level + 1) % 4;
			this->KeysProcessed[GLFW_KEY_W] = true;
		}
		if (this->Keys[GLFW_KEY_S] && !this->KeysProcessed[GLFW_KEY_S])
		{
			if (this->Level > 0)
				--this->Level;
			else
				this->Level = 3;
			//this->Level = (this->Level - 1) % 4;
			this->KeysProcessed[GLFW_KEY_S] = true;
		}
	}
	if (this->State == GAME_WIN)
	{
		if (this->Keys[GLFW_KEY_ENTER])
		{
			this->KeysProcessed[GLFW_KEY_ENTER] = true;
			Effects->Chaos = false;
			this->State = GAME_MENU;
		}
	}
	if (this->State == GAME_ACTIVE)
	{
		GLfloat velocity = PLAYER_VELOCITY * dt;
		// 移动挡板
		if (this->Keys[GLFW_KEY_A])
		{
			if (Player->Position.x >= 0)
			{
				Player->Position.x -= velocity;
				if (Ball->Stuck)
				{
					Ball->Position.x -= velocity;
				}
			}
				
		}
		if (this->Keys[GLFW_KEY_D])
		{
			if (Player->Position.x <= this->Width - Player->Size.x)
			{
				Player->Position.x += velocity;
				if (Ball->Stuck)
				{
					Ball->Position.x += velocity;
				}
			}
				
		}

		if (this->Keys[GLFW_KEY_SPACE])
			Ball->Stuck = false;

		if (this->Keys[GLFW_KEY_1])
		{	
			if (Effects != nullptr)
			{
				Effects->Chaos = true;
			}
		}
		if (this->Keys[GLFW_KEY_2])
		{
			if (Effects != nullptr)
			{
				Effects->Confuse = true;
			}
		}
	}
}

void Game::Update(GLfloat dt)
{
	Ball->Move(dt, this->Width);
	DoCollisions();

	Particles->Update(dt, *Ball, 2, glm::vec2(Ball->Radius / 2.0f));
	UpdatePowerUps(dt);
	if (ShakeTime > 0)
	{
		ShakeTime -= dt;
		if (ShakeTime <= 0.f)
		{
			Effects->Shake = false;
		}
	}

	if (Ball->Position.y >= this->Height) //球没接住，碰到了下面的边
	{
		--this->Lives;
		// did the player lose all his lives? : game over
		if (this->Lives == 0)
		{
			this->ResetLevel();
			this->State = GAME_MENU;
		}
		this->ResetPlayer();
	}

	if (this->State == GAME_ACTIVE && this->Levels[this->Level].IsCompleted())
	{
		this->ResetLevel();
		this->ResetPlayer();
		Effects->Chaos = true;
		this->State = GAME_WIN;
	}

}

void Game::Render()
{
	if (this->State == GAME_ACTIVE || this->State == GAME_MENU || this->State == GAME_WIN)
	{
		Effects->BeginRender();
			// draw background
			sprite->Draw(Resource::GetTexture("background"), glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f);
			// draw level
			this->Levels[this->Level].Draw(*sprite);
			// draw player
			Player->Draw(*sprite);
			for (PowerUp& powerUp : this->PowerUps)
			{
				if (!powerUp.Destroyed)
					powerUp.Draw(*sprite);
			}
		

			Particles->Draw();
			Ball->Draw(*sprite);
		Effects->EndRender();
		Effects->Render(glfwGetTime());

		std::stringstream ss, ss2, ss3;
		ss<<Lives;
		ss2<<Score;
		ss3<<MaxScore;
		Text->RenderText("Lives: " + ss.str(), 15.0f, 10.0f, 0.8f);
		Text->RenderText("Score: " + ss2.str(), 15.0f, 35.0f, 0.8f, glm::vec3(1.0, 1.0, 0.0));
		Text->RenderText("MaxScore: " + ss3.str(), 15.0f, 60.0f, 0.8f, glm::vec3(1.0,0.0,0.0));
	}
	if (this->State == GAME_MENU)
	{
		Text->RenderText("Press ENTER to start", 320, this->Height / 2.0f, 1.0f);
		Text->RenderText("Press W or S to select level", 320, this->Height / 2.0f + 30.0f, 0.75f);
	}
	if (this->State == GAME_WIN)
	{
		Text->RenderText("You WON!!!", 320.0f, this->Height / 2.0f - 20.0f, 1.0f, glm::vec3(0.0f, 1.0f, 0.0f));
		Text->RenderText("Press ENTER to retry or ESC to quit", 130.0f, this->Height / 2.0f, 1.0f, glm::vec3(1.0f, 1.0f, 0.0f));
	}
}

void Game::ResetLevel()
{
	if (this->Level == 0)
		this->Levels[0].Load("Resource/levels/one.lvl", this->Width, this->Height / 2);
	else if (this->Level == 1)
		this->Levels[1].Load("Resource/levels/two.lvl", this->Width, this->Height / 2);
	else if (this->Level == 2)
		this->Levels[2].Load("Resource/levels/three.lvl", this->Width, this->Height / 2);
	else if (this->Level == 3)
		this->Levels[3].Load("Resource/levels/four.lvl", this->Width, this->Height / 2);

	this->Lives = 3;
	this->Score = 0;
}

void Game::ResetPlayer()
{
	Player->Size = PLAYER_SIZE;
	Player->Position = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);
	Ball->Reset(Player->Position + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -(BALL_RADIUS * 2.0f)), INITIAL_BALL_VELOCITY);
}

bool ShouldSpawn(unsigned int chance)
{
	unsigned int random = rand() % chance;
	return random == 0;
}

void Game::SpawnPowerUps(GameObject& block)
{
	if (ShouldSpawn(75)) // 1 in 75 chance
		this->PowerUps.push_back(PowerUp("speed", glm::vec3(0.5f, 0.5f, 1.0f), 0.0f, block.Position, Resource::GetTexture("powerup_speed")));
	if (ShouldSpawn(75))
		this->PowerUps.push_back(PowerUp("sticky", glm::vec3(1.0f, 0.5f, 1.0f), 20.0f, block.Position, Resource::GetTexture("powerup_sticky")));
	if (ShouldSpawn(75))
		this->PowerUps.push_back(PowerUp("pass-through", glm::vec3(0.5f, 1.0f, 0.5f), 10.0f, block.Position, Resource::GetTexture("powerup_passthrough")));
	if (ShouldSpawn(75))
		this->PowerUps.push_back(PowerUp("pad-size-increase", glm::vec3(1.0f, 0.6f, 0.4), 0.0f, block.Position, Resource::GetTexture("powerup_increase")));
	if (ShouldSpawn(15)) // Negative powerups should spawn more often
		this->PowerUps.push_back(PowerUp("confuse", glm::vec3(1.0f, 0.3f, 0.3f), 15.0f, block.Position, Resource::GetTexture("powerup_confuse")));
	if (ShouldSpawn(15))
		this->PowerUps.push_back(PowerUp("chaos", glm::vec3(0.9f, 0.25f, 0.25f), 15.0f, block.Position, Resource::GetTexture("powerup_chaos")));
}
bool IsOtherPowerUpActive(std::vector<PowerUp>& powerUps, std::string type)
{
	// Check if another PowerUp of the same type is still active
	// in which case we don't disable its effect (yet)
	for (const PowerUp& powerUp : powerUps)
	{
		if (powerUp.Activated)
			if (powerUp.Type == type)
				return true;
	}
	return false;
}
void ActivatePowerUp(PowerUp& powerUp)
{
	if (powerUp.Type == "speed")
	{
		Ball->Velocity *= 1.2;
	}
	else if (powerUp.Type == "sticky")
	{
		Ball->Sticky = true;
		Player->Color = glm::vec3(1.0f, 0.5f, 1.0f);
	}
	else if (powerUp.Type == "pass-through")
	{
		Ball->PassThrough = true;
		Ball->Color = glm::vec3(1.0f, 0.5f, 0.5f);
	}
	else if (powerUp.Type == "pad-size-increase")
	{
		Player->Size.x += 50;
	}
	else if (powerUp.Type == "confuse")
	{
		if (!Effects->Chaos)
			Effects->Confuse = true; // only activate if chaos wasn't already active
	}
	else if (powerUp.Type == "chaos")
	{
		if (!Effects->Confuse)
			Effects->Chaos = true;
	}
}


void Game::UpdatePowerUps(float dt)
{
	for (PowerUp& powerUp : this->PowerUps)
	{
		powerUp.Position += powerUp.Velocity * dt;
		if (powerUp.Activated)
		{
			powerUp.Duration -= dt;

			if (powerUp.Duration <= 0.0f)
			{
				// remove powerup from list (will later be removed)
				powerUp.Activated = false;
				// deactivate effects
				if (powerUp.Type == "sticky")
				{
					if (!IsOtherPowerUpActive(this->PowerUps, "sticky"))
					{	// only reset if no other PowerUp of type sticky is active
						Ball->Sticky = false;
						Player->Color = glm::vec3(1.0f);
					}
				}
				else if (powerUp.Type == "pass-through")
				{
					if (!IsOtherPowerUpActive(this->PowerUps, "pass-through"))
					{	// only reset if no other PowerUp of type pass-through is active
						Ball->PassThrough = false;
						Ball->Color = glm::vec3(1.0f);
					}
				}
				else if (powerUp.Type == "confuse")
				{
					if (!IsOtherPowerUpActive(this->PowerUps, "confuse"))
					{	// only reset if no other PowerUp of type confuse is active
						Effects->Confuse = false;
					}
				}
				else if (powerUp.Type == "chaos")
				{
					if (!IsOtherPowerUpActive(this->PowerUps, "chaos"))
					{	// only reset if no other PowerUp of type chaos is active
						Effects->Chaos = false;
					}
				}
			}
		}
	}
	// Remove all PowerUps from vector that are destroyed AND !activated (thus either off the map or finished)
	// Note we use a lambda expression to remove each PowerUp which is destroyed and not activated
	this->PowerUps.erase(std::remove_if(this->PowerUps.begin(), this->PowerUps.end(),
		[](const PowerUp& powerUp) { return powerUp.Destroyed && !powerUp.Activated; }
	), this->PowerUps.end());
}
