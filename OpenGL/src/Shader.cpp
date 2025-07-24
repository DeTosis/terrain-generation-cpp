#include "Shader.h"
#include <GLFW/glfw3.h>

#include <iostream>

#define SHUTDOWN	glfwTerminate();\
					std::cin.get();\
					exit (-1);

Shader::Shader(std::string filePath)
	: m_FilePath(filePath)
{
	ShaderSrc src = ParseShader();

	if (!CompileShader(src)) 
	{
		SHUTDOWN;
	};
}

Shader::~Shader()
{
	glDeleteProgram(m_RendererId);
}

const unsigned int& Shader::GetProgram() const
{
	return m_RendererId;
}

void Shader::Bind() const
{
	glUseProgram(m_RendererId);
}

void Shader::UnBind() const
{
	glUseProgram(0);
}

void Shader::SetUniformMatrix4fv(const std::string& name, glm::mat4& matrix)
{
	glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]);
}

ShaderSrc Shader::ParseShader()
{
	std::ifstream stream(m_FilePath);
	std::stringstream buffer[2];
	std::string line;

	ShaderType type = ShaderType::UNDEF;

	if (!stream.is_open())
	{
		std::print("[ IO Exception ]Failed to open shader file, path {}", m_FilePath);
		SHUTDOWN;
	}

	while (std::getline(stream, line))
	{
		if (line.find("//") != std::string::npos)
			continue;
		if (line.empty())
			continue;

		if (line.starts_with("#shader"))
		{
			if (line.find("VERTEX") != std::string::npos)
			{
				type = ShaderType::VERTEX;
			}
			else if (line.find("FRAGMEN") != std::string::npos)
			{
				type = ShaderType::FRAGMENT;
			}
		}
		else
		{
			if (type != ShaderType::UNDEF)
			{
				buffer[(int)type] << line << "\n";
			}
		}
	}

	return {buffer[0].str(), buffer[1].str()};
}

bool Shader::CompileShader(ShaderSrc src)
{
	unsigned vertex = CreateShader(GL_VERTEX_SHADER, src.VertexSrc.c_str());
	unsigned fragment = CreateShader(GL_FRAGMENT_SHADER, src.FragmentSrc.c_str());

	unsigned int program = CreateProgram(vertex, fragment);
	
	m_RendererId = program;
	return true;
}

unsigned int Shader::CreateShader(const unsigned int type, const char *src)
{
	int compileStatus;
	char log[512];

	unsigned int shader; 
	switch (type)
	{
	case GL_VERTEX_SHADER:
		shader = glCreateShader(GL_VERTEX_SHADER);
		break;
	case GL_FRAGMENT_SHADER:
		shader = glCreateShader(GL_FRAGMENT_SHADER);
		break;
	default:
		std::println("[ Shader Error ] [ Shader creation ] Shader type is undefined or incorrect");
		SHUTDOWN;
	}

	glShaderSource(shader, 1, &src, nullptr);
	glCompileShader(shader);
	
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
	if (!compileStatus)
	{
		glGetShaderInfoLog(shader, 512, nullptr, log);
		std::println("[ Shader Error ] [ Shader creation ] [ Shader type: {} ] Msg: {}", 
			type == GL_VERTEX_SHADER ? "GL_VERTEX_SHADER" : "GL_FRAGMENT_SHADER", log);
		SHUTDOWN;
	}

	return shader;
}

unsigned int Shader::CreateProgram(const unsigned int vertex, const unsigned int fragment)
{
	int compileStatus;
	char log[512];

	unsigned int program;
	program = glCreateProgram();

	glAttachShader(program, vertex);
	glAttachShader(program, fragment);
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &compileStatus);
	if (!compileStatus)
	{
		glGetProgramInfoLog(program, 512, NULL, log);
		std::println("[ Shader Error ] [ Shader compilation ] [ Program Failed ] Msg: {}", log);
		SHUTDOWN;
	}

	return program;
}

int Shader::GetUniformLocation(const std::string& name)
{
	return glGetUniformLocation(m_RendererId, name.c_str());
}
