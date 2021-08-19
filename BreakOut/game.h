#pragma once


#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "glHeader.h"

class GameLevel;



// ��������Ϸ�ĵ�ǰ״̬
enum GameState {
	GAME_ACTIVE,
	GAME_MENU,
	GAME_WIN
};



class Game
{
public:

	// ��Ϸ״̬
	GameState  State;
	GLboolean  Keys[1024];
	GLuint     Width, Height;

	std::vector<GameLevel>  Levels;
	unsigned int            Level;

	// ��ҳߴ�
	const glm::vec2 PLAYER_SIZE  = glm::vec2(100, 20);
	// ����ٶ�
	const float PLAYER_VELOCITY = 500.0f;
	// ��ʼ����ٶ�
	const glm::vec2 INITIAL_BALL_VELOCITY = glm::vec2(100.0f, -350.0f);
	// ��뾶
	const float BALL_RADIUS = 12.5f;




	// ���캯��/��������
	Game(GLuint width, GLuint height);
	~Game();
	// ��ʼ����Ϸ״̬���������е���ɫ��/����/�ؿ���
	void Init();
	// ��Ϸѭ��
	void ProcessInput(GLfloat dt);
	void Update(GLfloat dt);
	void Render();
};

