#include "Resource.h"

#include "glHeader.h"

#include <iostream>
#include <sstream>
#include <fstream>


std::map<std::string, Shader> Resource::Shaders;
std::map<std::string, Texture2D> Resource::Textures;

Shader Resource::LoadShader(const GLchar* vShaderFile, const GLchar* fShaderFile, const GLchar* gShaderFile, std::string name)
{
	Shaders[name] = loadShaderFromFile(vShaderFile, fShaderFile, gShaderFile);
	return Shaders[name];
}

Shader& Resource::GetShader(std::string name)
{
	return Shaders[name];
}

Texture2D Resource::LoadTexture(const char* file, bool alpha, std::string name)
{
	Textures[name] = loadTextureFromFile(file, alpha);
	return Textures[name];
}

Texture2D& Resource::GetTexture(std::string name)
{
	return Textures[name];
}

void Resource::Clear()
{
	for (auto iter : Shaders)
		glDeleteProgram(iter.second.ID);
	for (auto iter : Textures)
		glDeleteTextures(1, &iter.second.ID);
}

Shader Resource::loadShaderFromFile(const GLchar* vShaderFile, const GLchar* fShaderFile, const GLchar* gShaderFile /*= nullptr*/)
{
	std::string vertexCode;
	std::string fragmentCode;
	std::string geometryCode;

	try
	{
		// open files
		std::ifstream vertexShaderFile(vShaderFile);
		std::ifstream fragmentShaderFile(fShaderFile);
		std::stringstream vShaderStream, fShaderStream;
		// read file's buffer contents into streams
		vShaderStream << vertexShaderFile.rdbuf();
		fShaderStream << fragmentShaderFile.rdbuf();
		// close file handlers
		vertexShaderFile.close();
		fragmentShaderFile.close();
		// convert stream into string
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
		// if geometry shader path is present, also load a geometry shader
		if (gShaderFile != nullptr)
		{
			std::ifstream geometryShaderFile(gShaderFile);
			std::stringstream gShaderStream;
			gShaderStream << geometryShaderFile.rdbuf();
			geometryShaderFile.close();
			geometryCode = gShaderStream.str();
		}
	}
	catch (std::exception e)
	{
		std::cout << "ERROR::SHADER: Failed to read shader files" << std::endl;
	}
	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();
	const char* gShaderCode = geometryCode.c_str();
	//// 2. now create shader object from source code
	Shader shader;
	shader.Compile(vShaderCode, fShaderCode, gShaderFile != nullptr ? gShaderCode : nullptr);
	return shader;

}

Texture2D Resource::loadTextureFromFile(const GLchar* file, GLboolean alpha)
{
	Texture2D texture;
	if (alpha)
	{
		texture.Internal_Format = GL_RGBA;
		texture.Image_Format = GL_RGBA;
	}
	// Load image
	int width, height, nrChannels;
	unsigned char* image = stbi_load(file, &width, &height, &nrChannels, 0);
	// Now generate texture
	texture.Generate(width, height, image);
	// And finally free image data
	stbi_image_free(image);
	return texture;
}
