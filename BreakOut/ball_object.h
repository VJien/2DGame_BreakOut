#pragma once
#include "glHeader.h"
#include "Texture.h"
#include "game_object.h"

class BallObject : public GameObject
{
public:
	//��״̬
	float   Radius;
	bool    Stuck;//�̶��ڵ���
	bool    Sticky;
	bool    PassThrough;

	BallObject();
	BallObject(glm::vec2 pos, GLfloat radius, glm::vec2 velocity, Texture2D sprite);

	glm::vec2 Move(GLfloat dt, GLuint window_width);
	void      Reset(glm::vec2 position, glm::vec2 velocity);


};

