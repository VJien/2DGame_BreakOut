#pragma once
#include "glHeader.h"
#include "Texture.h"
#include "game_object.h"

class BallObject : public GameObject
{
public:
	//Çò×´Ì¬
	float   Radius;
	bool    Stuck;//¹Ì¶¨ÔÚµ²°å
	bool    Sticky;
	bool    PassThrough;

	BallObject();
	BallObject(glm::vec2 pos, GLfloat radius, glm::vec2 velocity, Texture2D sprite);

	glm::vec2 Move(GLfloat dt, GLuint window_width);
	void      Reset(glm::vec2 position, glm::vec2 velocity);


};

