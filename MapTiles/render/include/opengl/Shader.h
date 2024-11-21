#pragma once

#include <string>
#include <glm.hpp>
#include <vector>

struct ShaderProgramSource
{
	std::string VertexSource;
	std::string FragmentSource;
};

// Opengl Shader abstraction class
class Shader
{
public:
	Shader(const std::string& vertexShader, const std::string& fragmentShader);
	~Shader();

	void Bind() const;
	void Unbind() const;
	void SetUniform1i(const std::string& name, int v);
	void SetUniform3f(const std::string& name, float v0, float v1, float v2);
	void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);
	void SetUniformMat4f(const std::string& name, glm::mat4& mat);

	static std::vector<std::string> ShaderDirectories;
private:
	ShaderProgramSource ParseShader() const;
	unsigned int CompileShader(unsigned int type, const std::string& source);
	unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader);

	int GetUniformLocation(const std::string& name) const;

private:
	unsigned int m_Id;
	std::string m_VSFilePath;
	std::string m_FSFilePath;
};