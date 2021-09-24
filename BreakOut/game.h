#pragma once


#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "glHeader.h"

class GameLevel;
class GameObject;
class PowerUp;



// ��������Ϸ�ĵ�ǰ״̬
enum GameState {
	GAME_ACTIVE,
	GAME_MENU,
	GAME_WIN
};
enum Direction {
	UP,
	RIGHT,
	DOWN,
	LEFT
};

//�Ƿ���ײ, ����, ��������
typedef std::tuple<bool, Direction, glm::vec2> Collision;


class Game
{
public:

	// ��Ϸ״̬
	GameState  State;
	GLboolean  Keys[1024];
	GLuint     Width, Height;

	std::vector<GameLevel>  Levels;
	unsigned int            Level;
	unsigned int            Lives;
	unsigned int            Score;
	unsigned int            MaxScore;
	bool                    KeysProcessed[1024];
	std::vector<PowerUp>    PowerUps;


	// ��ҳߴ�
	const glm::vec2 PLAYER_SIZE  = glm::vec2(100, 20);
	// ����ٶ�
	const float PLAYER_VELOCITY = 500.0f;
	// ��ʼ����ٶ�
	const glm::vec2 INITIAL_BALL_VELOCITY = glm::vec2(100.0f, -350.0f);
	// ��뾶
	const float BALL_RADIUS = 12.5f;

	

	void DoCollisions();

	// ���캯��/��������
	Game(GLuint width, GLuint height);
	~Game();
	// ��ʼ����Ϸ״̬���������е���ɫ��/����/�ؿ���
	void Init();
	// ��Ϸѭ��
	void ProcessInput(GLfloat dt);
	void Update(GLfloat dt);
	void Render();

	void ResetLevel();
	void ResetPlayer();


	void SpawnPowerUps(GameObject& block);
	void UpdatePowerUps(float dt);
};

