#include "game.h"
#include "Sprite2D.h"
#include "Resource.h"
#include "game_level.h"
#include "ball_object.h"
#include "game_object.h"



Sprite2D* sprite; 
GameObject* Player;
BallObject* Ball;

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
	//aabb�İ볤
	glm::vec2 aabb_half_extents(two.Size.x / 2.0f, two.Size.y / 2.0f);
	//aabb������λ��
	glm::vec2 aabb_center(two.Position.x + aabb_half_extents.x, two.Position.y + aabb_half_extents.y);
	// ��ȡ�������ĵĲ�ʸ��
	glm::vec2 difference = center - aabb_center;

	glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
	// AABB_center����clamped�����͵õ�����ײ���Ͼ���Բ����ĵ�closest
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
			if (std::get<0>(collision)) // ���collision �� true
			{
				// ���ש�鲻��ʵ�ľ�����ש��
				if (!box.IsSolid)
					box.Destroyed = GL_TRUE;
				// ��ײ����
				Direction dir = std::get<1>(collision);
				glm::vec2 diff_vector = std::get<2>(collision);

				if (dir == LEFT || dir == RIGHT) // ˮƽ������ײ
				{
					Ball->Velocity.x = -Ball->Velocity.x; // ��תˮƽ�ٶ�
					// �ض�λ
					GLfloat penetration = Ball->Radius - std::abs(diff_vector.x);
					if (dir == LEFT)
						Ball->Position.x += penetration; // ��������
					else
						Ball->Position.x -= penetration; // ��������
				}
				else // ��ֱ������ײ
				{
					Ball->Velocity.y = -Ball->Velocity.y; // ��ת��ֱ�ٶ�
					// �ض�λ
					GLfloat penetration = Ball->Radius - std::abs(diff_vector.y);
					if (dir == UP)
						Ball->Position.y -= penetration; // ��������
					else
						Ball->Position.y += penetration; // ��������
				}

			}

			Collision result = CheckCollision(*Ball, *Player);
			if (!Ball->Stuck && std::get<0>(result))
			{
				// ��������˵�����ĸ�λ�ã������������ĸ�λ�����ı��ٶ�
				GLfloat centerBoard = Player->Position.x + Player->Size.x / 2;
				GLfloat distance = (Ball->Position.x + Ball->Radius) - centerBoard;
				GLfloat percentage = distance / (Player->Size.x / 2);
				// ���ݽ���ƶ�
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
}

void Game::Init()
{
	// ������ɫ��
	Resource::LoadShader("Shaders/sprite.vert", "Shaders/sprite.frag", nullptr, "sprite");
	glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(this->Width),
		static_cast<GLfloat>(this->Height), 0.0f, -1.0f, 1.0f);
	Resource::GetShader("sprite").Use().SetInteger("image", 0);
	Resource::GetShader("sprite").SetMatrix4("projection", projection);

	// ����ר������Ⱦ�Ŀ���
	sprite = new Sprite2D(Resource::GetShader("sprite"));

	// ��������
	Resource::LoadTexture("Resource/Textures/background.jpg", GL_FALSE, "background");
	Resource::LoadTexture("Resource/Textures/awesomeface.png", GL_TRUE, "face");
	Resource::LoadTexture("Resource/Textures/block.png", GL_FALSE, "block");
	Resource::LoadTexture("Resource/Textures/block_solid.png", GL_FALSE, "block_solid");
	Resource::LoadTexture("Resource/Textures/paddle.png", GL_TRUE, "paddle");


	// ���عؿ�
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
		// �ƶ�����
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
	}
}

void Game::Update(GLfloat dt)
{
	Ball->Move(dt, this->Width);
	DoCollisions();
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
