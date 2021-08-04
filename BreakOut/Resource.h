#pragma once

#include <map>
#include <string>

#include "glad/glad.h"
#include "Texture.h"
#include "Shader.h"


class Resource
{
public:

	
	static std::map<std::string, Shader>    Shaders;
	static std::map<std::string, Texture2D> Textures;

	static Shader LoadShader(const GLchar* vShaderFile, const GLchar* fShaderFile, const GLchar* gShaderFile, std::string name);
	static Shader& GetShader(std::string name);
	static Texture2D LoadTexture(const char* file, bool alpha, std::string name);
	static Texture2D& GetTexture(std::string name);
	// 清理所有加载的内容
	static void Clear();
private:
	Resource() { }
	//从文件加载Shader
	static Shader    loadShaderFromFile(const GLchar* vShaderFile, const GLchar* fShaderFile, const GLchar* gShaderFile = nullptr);
	//从文件加载图片
	static Texture2D loadTextureFromFile(const GLchar* file, GLboolean alpha);
};

