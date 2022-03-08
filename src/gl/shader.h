#ifndef SHADER_OBJECT_H
#define SHADER_OBJECT_H

#include <GL/glew.h>
#include <vector>
#include <string>

namespace gl
{

using std::vector;
using std::string;

/**
* A simplified class to handle OpenGL shaders
*/
class Shader
{
private:
	/**
	* The code of the shader
	*/
	vector<string> _codeLines;

	/**
	* OpenGL id of the shader
	*/
	GLuint _idxShaderGL;

	/**
	* OpenGL id of the compiled shader
	*/
	GLuint _idxProgramGL;

	/**
	* TRUE if the shader is compiled
	*/
	bool _compiled;

	/**
	* TRUE if the shader is attached
	*/
	bool _attached;

	const char** lineArray() const;
public:
	/**
	* Output Log after trying to compile the shader
	*/
	struct Log
	{
		/**
		* Compiler output
		*/
		char* info;

		/**
		* Compiler output length
		*/
		int  length;
		Log& operator=(const Shader::Log& log);
		Log();
		Log(const Shader::Log& log);
		~Log();
	};

	Shader();
	~Shader();

	/**
	* Loads a file containing the shader code
	* @param filename File name of the shader code
	*/
	void Load_File(const string& filename);

	/**
	* Compiles the shader as a fragment shader
	*/
	Shader::Log Compile_Fragment();

	/**
	* Com[piles the shader as a vertex shader
	*/
	Shader::Log Compile_Vertex();

	/**
	* Attaches the current shader to the OpenGL rendering pipeline
	*/
	void Attach();

	/**
	* Attaches a program to the current OpenGL rendering pipeling and
	* the current shader
	*/
	void Attach(GLuint programid);

	/**
	* Attaches another shader to the current OpenGL rendering pipeling and
	* the current shader
	*/
	void Attach(const Shader& vertexshader);

	/**
	* Executes current shader
	*/
	Shader::Log Execute();

	/**
	* @return OpenGL id of the shader (associated to the compiled program)
	*/
	GLuint ProgramIDX() const;

	/**
	* @return OpenGL id of the shader 
	*/
	GLuint ShaderIDX() const;

	/**
	* @return TRUE if the program was succesfully compiled
	*/
	bool Compiled() const;

	/**
	* Deteaches shader from the rendering pipeline, erasing all memory
	*/
	void Detach();
};

inline GLuint Shader::ProgramIDX() const
{
	return(this->_idxProgramGL);
}

inline GLuint Shader::ShaderIDX() const
{
	return(this->_idxShaderGL);
}

inline bool Shader::Compiled() const
{
	return(this->_compiled);
}

}

#endif