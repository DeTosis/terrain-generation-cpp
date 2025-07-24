#pragma once
#define GLEW_STATIC
#include <GL/glew.h>

#include "glm/glm.hpp"

#include <fstream>
#include <string>
#include <sstream>
#include <format>

#include <print>
#include <expected>

enum class ShaderType
{
	UNDEF = -1,
	VERTEX = 0,
	FRAGMENT = 1,
};

struct ShaderSrc
{
	std::string VertexSrc;
	std::string FragmentSrc;
};

class Shader
{
private:
	unsigned int m_RendererId;
	std::string m_FilePath;
public:
	Shader(std::string filePath);
	~Shader();

	void Bind() const; 
	void UnBind() const; 
public:
	void SetUniformMatrix4fv(const std::string& name, glm::mat4& matrix);
private:
	ShaderSrc ParseShader();
	bool CompileShader(ShaderSrc src);

	unsigned int CreateShader(const unsigned int type, const char* src);
	unsigned int CreateProgram( const unsigned int vertexShader, const unsigned int fragmentShader);

	const unsigned int& GetProgram() const;
	int GetUniformLocation(const std::string& name);
};