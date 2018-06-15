
#include "Program.h"
#include <iostream>
#include <cassert>
#include <fstream>

#include "GLSL.h"


std::string readFileAsString(const std::string &fileName)
{
	std::string result;
	std::ifstream fileHandle(fileName);

	fileHandle.seekg(0, std::ios::end);
	result.reserve((size_t) fileHandle.tellg());
	fileHandle.seekg(0, std::ios::beg);

	result.assign((std::istreambuf_iterator<char>(fileHandle)), std::istreambuf_iterator<char>());

	return result;
}

// Using this overload will skip all of the tessellation shader initialization and link your vertex shader directly to your fragment shader
void Program::setShaderNames(const std::string &v, const std::string &f) {
	usingTess = 0;
	vShaderName = v;
	fShaderName = f;
}

// This overload will initialize tessellation shaders, and link your vertex shader to the tessellation control shader
void Program::setShaderNames(const std::string &v, const std::string &f, const std::string &tc, const std::string &te)
{
	usingTess = 1;
	vShaderName = v;
	fShaderName = f;
	teShaderName = te;
	tcShaderName = tc;
}

bool Program::init()
{
	GLint rc;

	// Create shader handles
	GLuint VS = glCreateShader(GL_VERTEX_SHADER);
	GLuint FS = glCreateShader(GL_FRAGMENT_SHADER);

	GLuint TC = glCreateShader(GL_TESS_CONTROL_SHADER);
	GLuint TE = glCreateShader(GL_TESS_EVALUATION_SHADER);



	// Read shader sources
	std::string vShaderString = readFileAsString(vShaderName);
	std::string fShaderString = readFileAsString(fShaderName);
	const char *vshader = vShaderString.c_str();
	const char *fshader = fShaderString.c_str();
	CHECKED_GL_CALL(glShaderSource(VS, 1, &vshader, NULL));
	CHECKED_GL_CALL(glShaderSource(FS, 1, &fshader, NULL));


	// TESS vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	if (usingTess == 1) {
		std::string teShaderString = readFileAsString(teShaderName);
		std::string tcShaderString = readFileAsString(tcShaderName);
		const char *teshader = teShaderString.c_str();
		const char *tcshader = tcShaderString.c_str();

		CHECKED_GL_CALL(glShaderSource(TE, 1, &teshader, NULL));
		CHECKED_GL_CALL(glShaderSource(TC, 1, &tcshader, NULL));
	}
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


	// Compile vertex shader
	CHECKED_GL_CALL(glCompileShader(VS));
	CHECKED_GL_CALL(glGetShaderiv(VS, GL_COMPILE_STATUS, &rc));
	if (!rc)
	{
		if (isVerbose())
		{
			GLSL::printShaderInfoLog(VS);
			std::cout << "Error compiling vertex shader " << vShaderName << std::endl;
		}
		return false;
	}

	// TESS vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	// Compile TE
	if (usingTess) {
		CHECKED_GL_CALL(glCompileShader(TE));
		CHECKED_GL_CALL(glGetShaderiv(TE, GL_COMPILE_STATUS, &rc));
		if (!rc)
		{
			if (isVerbose())
			{
				GLSL::printShaderInfoLog(TE);
				std::cout << "Error compiling TE shader " << teShaderName << std::endl;
			}
			return false;
		}

		// Compile TC
		CHECKED_GL_CALL(glCompileShader(TC));
		CHECKED_GL_CALL(glGetShaderiv(TC, GL_COMPILE_STATUS, &rc));
		if (!rc)
		{
			if (isVerbose())
			{
				GLSL::printShaderInfoLog(TC);
				std::cout << "Error compiling TC shader " << tcShaderName << std::endl;
			}
			return false;
		}
	}
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	// Compile fragment shader
	CHECKED_GL_CALL(glCompileShader(FS));
	CHECKED_GL_CALL(glGetShaderiv(FS, GL_COMPILE_STATUS, &rc));
	if (!rc)
	{
		if (isVerbose())
		{
			GLSL::printShaderInfoLog(FS);
			std::cout << "Error compiling fragment shader " << fShaderName << std::endl;
		}
		return false;
	}

	// Create the program and link
	pid = glCreateProgram();
	CHECKED_GL_CALL(glAttachShader(pid, VS));
	CHECKED_GL_CALL(glAttachShader(pid, FS));

	// TESS vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	if (usingTess) {
		CHECKED_GL_CALL(glAttachShader(pid, TC));
		CHECKED_GL_CALL(glAttachShader(pid, TE));
	}
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	CHECKED_GL_CALL(glLinkProgram(pid));
	CHECKED_GL_CALL(glGetProgramiv(pid, GL_LINK_STATUS, &rc));
	if (!rc)
	{
		if (isVerbose())
		{
			GLSL::printProgramInfoLog(pid);
			std::cout << "Error linking shaders " << vShaderName << " and " << fShaderName << std::endl;
		}
		return false;
	}

	return true;
}

void Program::bind()
{
	CHECKED_GL_CALL(glUseProgram(pid));
}

void Program::unbind()
{
	CHECKED_GL_CALL(glUseProgram(0));
}

void Program::addAttribute(const std::string &name)
{
	attributes[name] = GLSL::getAttribLocation(pid, name.c_str(), isVerbose());
}

void Program::addUniform(const std::string &name)
{
	uniforms[name] = GLSL::getUniformLocation(pid, name.c_str(), isVerbose());
}

GLint Program::getAttribute(const std::string &name) const
{
	std::map<std::string, GLint>::const_iterator attribute = attributes.find(name.c_str());
	if (attribute == attributes.end())
	{
		if (isVerbose())
		{
			std::cout << name << " is not an attribute variable" << std::endl;
		}
		return -1;
	}
	return attribute->second;
}

GLint Program::getUniform(const std::string &name) const
{
	std::map<std::string, GLint>::const_iterator uniform = uniforms.find(name.c_str());
	if (uniform == uniforms.end())
	{
		if (isVerbose())
		{
			std::cout << name << " is not a uniform variable" << std::endl;
		}
		return -1;
	}
	return uniform->second;
}
