#pragma once

#include "glHeader.h"
#include "Texture.h"


//���ں���
class PostProcessor
{
public:
	Shader PostProcessingShader;

	Texture2D Texture;

	unsigned int Width, Height;
	bool Confuse, Chaos, Shake;

	PostProcessor(Shader shader, unsigned int width, unsigned int height);

	// �ڳ�����Ϸ֮ǰ׼����������framebuffer����
	void BeginRender();
	// Ӧ������Ⱦ��Ϸ����ã���������������Ⱦ���ݴ洢��һ�����������
	void EndRender();
	// ��ȾPostProcessor������֡(��Ϊһ��������Ļ�Ĵ��;���)
	void Render(float time);



private:
	// render state
	unsigned int MSFBO, FBO; // MSFBO =�����FBO��FBO�ǹ���ģ���������MSɫ���嵽����
	unsigned int RBO; // RBO ���ڶ������ɫ����
	unsigned int VAO;
	// initialize quad for rendering postprocessing texture
	void initRenderData();
};

