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
	//���ļ���ȡ�ؿ�
	void Load(const char* file, unsigned int levelWidth, unsigned int levelHeight);
	//���ƹؿ�
	void Draw(Sprite2D& sprite);
	//�ؿ��Ƿ����
	bool IsCompleted();
private:
	// ��ʼ���ؿ�����
	void init(std::vector<std::vector<unsigned int>> tileData, unsigned int levelWidth, unsigned int levelHeight);
};

