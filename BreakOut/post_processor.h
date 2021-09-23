#pragma once

#include "glHeader.h"
#include "Texture.h"


//后期盒子
class PostProcessor
{
public:
	Shader PostProcessingShader;

	Texture2D Texture;

	unsigned int Width, Height;
	bool Confuse, Chaos, Shake;

	PostProcessor(Shader shader, unsigned int width, unsigned int height);

	// 在呈现游戏之前准备后处理器的framebuffer操作
	void BeginRender();
	// 应该在渲染游戏后调用，所以它将所有渲染数据存储到一个纹理对象中
	void EndRender();
	// 渲染PostProcessor纹理四帧(作为一个包含屏幕的大型精灵)
	void Render(float time);



private:
	// render state
	unsigned int MSFBO, FBO; // MSFBO =多采样FBO。FBO是规则的，用于屏蔽MS色缓冲到纹理
	unsigned int RBO; // RBO 用于多采样颜色缓冲
	unsigned int VAO;
	// initialize quad for rendering postprocessing texture
	void initRenderData();
};

