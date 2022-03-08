#include <stdio.h>
#include <fstream>
#include <stdexcept>
#include "shader.h"

namespace gl
{

using std::ifstream;

Shader::Log& Shader::Log::operator=(const Shader::Log& log)
{
	if (this->info)
	{
		delete[]this->info;
	}
	this->info = new char[log.length];
	for (int i = 0; i < log.length; ++i)
	{
		this->info[i] = log.info[i];
	}
	this->length = log.length;
	return(*this);
}

Shader::Log::Log()
{
	this->info = 0;
	this->length = 0;
}

void Shader::Attach(const Shader& vertexshader)
{
	this->Attach(vertexshader.ProgramIDX());
}

Shader::Log::Log(const Shader::Log& log)
{
	if (log.length > 1)
	{
		this->info = new char[log.length];
		for (int i = 0; i < log.length; ++i)
		{
			this->info[i] = log.info[i];
		}
		this->length = log.length;
	}
	else
	{
		this->info = 0;
		this->length = 0;
	}
}

Shader::Log::~Log()
{
	if (this->info)
	{
		delete[]info;
	}
}


Shader::Shader()
{
	this->_idxShaderGL = 0;
	this->_idxProgramGL = 0;
	this->_compiled = false;
	this->_attached = false;
}

Shader::~Shader()
{
	this->Detach();
}

void Shader::Detach()
{
	if (this->_compiled)
	{
		glDetachShader(this->_idxProgramGL, this->_idxShaderGL);
		glDeleteShader(this->_idxShaderGL);
		glDeleteProgram(this->_idxProgramGL);
		this->_compiled = false;
		this->_attached = false;
		this->_idxShaderGL = 0;
		this->_idxProgramGL = 0;
		this->_codeLines.clear();
	}
}

void Shader::Load_File(const string& filename)
{
	this->_codeLines.clear();
	ifstream file;
	file.open(filename, std::ios_base::in);
	while (!file.eof())
	{
		string line;
		std::getline(file, line);
		line = line + "\n";
		this->_codeLines.push_back(line);
	}
	file.close();
}

const char** Shader::lineArray() const
{
	const char** lines = new const char*[this->_codeLines.size()];
	for (int i = 0; i < this->_codeLines.size(); ++i)
	{
		lines[i] = this->_codeLines[i].c_str();
	}
	return(lines);
}

Shader::Log Shader::Compile_Fragment()
{
	this->_idxShaderGL = glCreateShader(GL_FRAGMENT_SHADER);
	const char** lines = this->lineArray();
	glShaderSource(this->_idxShaderGL, (GLsizei)this->_codeLines.size(), lines, 0);
	glCompileShader(this->_idxShaderGL);

	Shader::Log out;
	int charsWritten;

	GLint l;
	glGetShaderiv(this->_idxShaderGL, GL_INFO_LOG_LENGTH, &l);
	out.length = (int)l;
	if (out.length > 1)
	{
		out.info = new char[out.length];
		glGetShaderInfoLog(this->_idxShaderGL, out.length, &charsWritten, out.info);
	}
	GLint i;
	glGetShaderiv(this->_idxShaderGL, GL_COMPILE_STATUS, &i);
	if (i == GL_FALSE)
	{
		printf(out.info);
		throw std::runtime_error("SHADER IS NOT COMPILED");
	}
	else
	{
		this->_compiled = true;
	}
	delete[]lines;
	return(out);
}

Shader::Log Shader::Compile_Vertex()
{
	this->_idxShaderGL = glCreateShader(GL_VERTEX_SHADER);
	const char** lines = this->lineArray();
	glShaderSource(this->_idxShaderGL, (GLsizei)this->_codeLines.size(), lines,0);
	glCompileShader(this->_idxShaderGL);

	Shader::Log out;
	int charsWritten;

	GLint l;
	glGetShaderiv(this->_idxShaderGL, GL_INFO_LOG_LENGTH, &l);
	out.length = (int)l;
	if (out.length > 1)
	{
		out.info = new char[out.length];
		glGetShaderInfoLog(this->_idxShaderGL, out.length, &charsWritten, out.info);
	}
	GLint i;
	glGetShaderiv(this->_idxShaderGL, GL_COMPILE_STATUS, &i);
	if (i == GL_FALSE)
	{
		string error = out.info;
		throw std::runtime_error(out.info);
	}
	else
	{
		this->_compiled = true;
	}
	delete[]lines;
	return(out);
}

void Shader::Attach()
{
	Shader::Log out;
	this->_idxProgramGL = glCreateProgram();
	glAttachShader(this->_idxProgramGL, this->_idxShaderGL);
	this->_attached = true;
}

Shader::Log Shader::Execute()
{
	Shader::Log out;
	glLinkProgram(this->_idxProgramGL);
	glUseProgram(this->_idxProgramGL);

	int charsWritten;

	GLint i;
	glGetProgramiv(this->_idxProgramGL, GL_INFO_LOG_LENGTH, &i);
	out.length = (int)i;
	if (out.length > 1)
	{
		out.info = new char[out.length];
		glGetProgramInfoLog(this->_idxProgramGL, out.length, &charsWritten, out.info);
	}
	else
	{
		this->_compiled = true;
	}
	return(out);
}

void Shader::Attach(GLuint programid)
{
	this->_idxProgramGL = programid;
	glAttachShader(this->_idxProgramGL, this->_idxShaderGL);
	this->_attached = true;
}



}