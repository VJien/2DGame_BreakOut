#include "game.h"
#include "Sprite2D.h"
#include "Resource.h"
#include "game_level.h"
#include "ball_object.h"



Sprite2D* sprite; 
GameObject* Player;
BallObject* Ball;

Game::Game(GLuint width, GLuint height)
	: State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{

}

Game::~Game()
{
	delete sprite;
	delete Player;
	delete Ball;
}

void Game::Init()
{
	// 加载着色器
	Resource::LoadShader("Shaders/sprite.vert", "Shaders/sprite.frag", nullptr, "sprite");
	glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(this->Width),
		static_cast<GLfloat>(this->Height), 0.0f, -1.0f, 1.0f);
	Resource::GetShader("sprite").Use().SetInteger("image", 0);
	Resource::GetShader("sprite").SetMatrix4("projection", projection);

	// 设置专用于渲染的控制
	sprite = new Sprite2D(Resource::GetShader("sprite"));

	// 加载纹理
	Resource::LoadTexture("Resource/Textures/background.jpg", GL_FALSE, "background");
	Resource::LoadTexture("Resource/Textures/awesomeface.png", GL_TRUE, "face");
	Resource::LoadTexture("Resource/Textures/block.png", GL_FALSE, "block");
	Resource::LoadTexture("Resource/Textures/block_solid.png", GL_FALSE, "block_solid");
	Resource::LoadTexture("Resource/Textures/paddle.png", GL_TRUE, "paddle");


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
				Player->Position.x -= velocity;
		}
		if (this->Keys[GLFW_KEY_D])
		{
			if (Player->Position.x <= this->Width - Player->Size.x)
				Player->Position.x += velocity;
		}

		if (this->Keys[GLFW_KEY_SPACE])
			Ball->Stuck = false;
	}
}

void Game::Update(GLfloat dt)
{
	Ball->Move(dt, this->Width);
}

void Game::Render()
{
	if (this->State == GAME_ACTIVE)
	{
		// draw background
		sprite->Draw(Resource::GetTexture("background"), glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f);
		// draw level
		this->Levels[this->Level].Draw(*sprite);
		// draw player
		Player->Draw(*sprite);

		Ball->Draw(*sprite);
	}
}
