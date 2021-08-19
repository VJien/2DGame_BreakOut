#pragma once

#include "glHeader.h"
#include "game_object.h"
#include "Sprite2D.h"
#include "Resource.h"
#include <vector>


class GameLevel
{
public:
	std::vector<GameObject> Bricks;

	GameLevel() { }
	//从文件读取关卡
	void Load(const char* file, unsigned int levelWidth, unsigned int levelHeight);
	//绘制关卡
	void Draw(Sprite2D& sprite);
	//关卡是否完成
	bool IsCompleted();
private:
	// 初始化关卡数据
	void init(std::vector<std::vector<unsigned int>> tileData, unsigned int levelWidth, unsigned int levelHeight);
};

