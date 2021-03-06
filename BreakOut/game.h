#pragma once


#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "glHeader.h"

class GameLevel;
class GameObject;
class PowerUp;



// 代表了游戏的当前状态
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

//是否碰撞, 方向, 中心向量
typedef std::tuple<bool, Direction, glm::vec2> Collision;


class Game
{
public:

	// 游戏状态
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


	// 玩家尺寸
	const glm::vec2 PLAYER_SIZE  = glm::vec2(100, 20);
	// 玩家速度
	const float PLAYER_VELOCITY = 500.0f;
	// 初始球的速度
	const glm::vec2 INITIAL_BALL_VELOCITY = glm::vec2(100.0f, -350.0f);
	// 球半径
	const float BALL_RADIUS = 12.5f;

	

	void DoCollisions();

	// 构造函数/析构函数
	Game(GLuint width, GLuint height);
	~Game();
	// 初始化游戏状态（加载所有的着色器/纹理/关卡）
	void Init();
	// 游戏循环
	void ProcessInput(GLfloat dt);
	void Update(GLfloat dt);
	void Render();

	void ResetLevel();
	void ResetPlayer();


	void SpawnPowerUps(GameObject& block);
	void UpdatePowerUps(float dt);
};

