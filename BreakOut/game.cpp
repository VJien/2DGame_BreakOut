#include "game.h"
#include "Sprite2D.h"
#include "Resource.h"



Sprite2D* sp; 


Game::Game(GLuint width, GLuint height)
	: State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{

}

Game::~Game()
{
	delete sp;
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
	sp = new Sprite2D(Resource::GetShader("sprite"));
	// 加载纹理
	Resource::LoadTexture("Resource/Textures/awesomeface.png", GL_TRUE, "face");


}

void Game::ProcessInput(GLfloat dt)
{

}

void Game::Update(GLfloat dt)
{

}

void Game::Render()
{
	sp->Draw(Resource::GetTexture("face"), glm::vec2(200.0f, 200.0f), glm::vec2(300.0f, 400.0f), 45.0f, glm::vec3(0.0f, 1.0f, 0.0f));
}
