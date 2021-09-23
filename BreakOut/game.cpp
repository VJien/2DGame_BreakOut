#include "game.h"
#include "Sprite2D.h"
#include "Resource.h"
#include "game_level.h"
#include "ball_object.h"
#include "game_object.h"
#include "Particle_Generator.h"
#include "post_processor.h"



Sprite2D* sprite; 
GameObject* Player;
BallObject* Ball;
ParticleGenerator* Particles;
PostProcessor* Effects;
float ShakeTime = 0.0f;

bool CheckCollision(GameObject& one, GameObject& two);
Collision CheckCollision(BallObject& one, GameObject& two);
Direction VectorDirection(glm::vec2 target);


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
					box.Destroyed = GL_TRUE;
				else
				{
					ShakeTime = 0.1f;
					Effects->Shake = true;
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

			}
		}
	}
}

Game::Game(GLuint width, GLuint height)
	: State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{

}

Game::~Game()
{
	delete sprite;
	delete Player;
	delete Ball;
	delete Particles;
	delete Effects;
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


	// 设置专用于渲染的控制
	sprite = new Sprite2D(Resource::GetShader("sprite"));
	



	// 加载纹理
	Resource::LoadTexture("Resource/Textures/background.jpg", GL_FALSE, "background");
	Resource::LoadTexture("Resource/Textures/awesomeface.png", GL_TRUE, "face");
	Resource::LoadTexture("Resource/Textures/block.png", GL_FALSE, "block");
	Resource::LoadTexture("Resource/Textures/block_solid.png", GL_FALSE, "block_solid");
	Resource::LoadTexture("Resource/Textures/paddle.png", GL_TRUE, "paddle");
	Resource::LoadTexture("Resource/Textures/particle.png", true, "particle");//粒子贴图

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

	//粒子发射器
	Particles = new ParticleGenerator(Resource::GetShader("particle"), Resource::GetTexture("particle"), 500);

	Effects = new PostProcessor(Resource::GetShader("postprocessing"), this->Width, this->Height);

}

void Game::ProcessInput(GLfloat dt)
{
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
	if (ShakeTime > 0)
	{
		ShakeTime -= dt;
		if (ShakeTime <= 0.f)
		{
			Effects->Shake = false;
		}
	}

	if (Ball->Position.y >= this->Height) // did ball reach bottom edge?
	{
		this->ResetLevel();
		this->ResetPlayer();
	}

}

void Game::Render()
{
	if (this->State == GAME_ACTIVE)
	{
		Effects->BeginRender();
		// draw background
		sprite->Draw(Resource::GetTexture("background"), glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f);
		// draw level
		this->Levels[this->Level].Draw(*sprite);
		// draw player
		Player->Draw(*sprite);
		Particles->Draw();
		Ball->Draw(*sprite);
		Effects->EndRender();

		Effects->Render(glfwGetTime());
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
}

void Game::ResetPlayer()
{
	Player->Size = PLAYER_SIZE;
	Player->Position = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);
	Ball->Reset(Player->Position + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -(BALL_RADIUS * 2.0f)), INITIAL_BALL_VELOCITY);
}
