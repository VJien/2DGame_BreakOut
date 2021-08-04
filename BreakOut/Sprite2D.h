#pragma once

#include "glHeader.h"
#include "Shader.h"
#include "Texture.h"



class Sprite2D
{
public:
	Sprite2D(Shader& shader);
	~Sprite2D();

	void Draw(Texture2D& texture, glm::vec2 position, glm::vec2 size = glm::vec2(10.0f, 10.0f), float rotate = 0.0f, glm::vec3 color = glm::vec3(1.0f));

private:
	Shader       _shader;
	unsigned int _quadVAO;
	// Initializes and configures the quad's buffer and vertex attributes
	void initRenderData();
};

