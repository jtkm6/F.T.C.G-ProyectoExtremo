#pragma once

// STD. Includes
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

// GLEW
#include <GL/glew.h>

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Other Libs
#include <Windows.h>

class Shader{
private:
	GLuint Program;

public:
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath){
		std::string vertexCode;
		std::string fragmentCode;
		std::ifstream vShaderFile;
		std::ifstream fShaderFile;
		vShaderFile.exceptions(std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::badbit);
		try{
			vShaderFile.open(vertexPath);
			fShaderFile.open(fragmentPath);
			std::stringstream vShaderStream, fShaderStream;
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			vShaderFile.close();
			fShaderFile.close();
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
		} catch(std::ifstream::failure e){
			OutputDebugStringA("ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ\n");
		}
		const GLchar* vShaderCode = vertexCode.c_str();
		const GLchar * fShaderCode = fragmentCode.c_str();
		GLuint vertex, fragment;
		GLint success;
		GLchar infoLog[512];
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
		if(!success){
			glGetShaderInfoLog(vertex, 512, NULL, infoLog);
			OutputDebugStringA("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n");
			OutputDebugStringA(infoLog);
		}
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
		if(!success){
			glGetShaderInfoLog(fragment, 512, NULL, infoLog);
			OutputDebugStringA("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n");
			OutputDebugStringA(infoLog);
		}
		this->Program = glCreateProgram();
		glAttachShader(this->Program, vertex);
		glAttachShader(this->Program, fragment);
		glLinkProgram(this->Program);
		glGetProgramiv(this->Program, GL_LINK_STATUS, &success);
		if(!success){
			glGetProgramInfoLog(this->Program, 512, NULL, infoLog);
			OutputDebugStringA("ERROR::SHADER::PROGRAM::LINKING_FAILED\n");
			OutputDebugStringA(infoLog);
		} else{
			OutputDebugStringA("SUCCES::SHADER::SHADER_FULL_LOADED\n");
		}
		glDeleteShader(vertex);
		glDeleteShader(fragment);

	}

	void Enable(){
		glUseProgram(Program);
	}

	void Disable(){
		glUseProgram(0);
	}

	void SetglUniformValue(GLchar* attributeName, glm::mat4 &value){
		GLint location = glGetUniformLocation(Program, attributeName);
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
	}

	void SetglUniformValue(GLchar* attributeName, glm::vec4 &value){
		GLint location = glGetUniformLocation(Program, attributeName);
		glUniform4fv(location, 1, glm::value_ptr(value));
	}

	void SetglUniformValue(GLchar* attributeName, glm::vec3 &value){
		GLint location = glGetUniformLocation(Program, attributeName);
		glUniform3fv(location, 1, glm::value_ptr(value));
	}

	void SetglUniformValue(GLchar* attributeName, glm::vec2 &value){
		GLint location = glGetUniformLocation(Program, attributeName);
		glUniform2fv(location, 1, glm::value_ptr(value));
	}

	void SetglUniformValue(GLchar* attributeName, GLfloat value){
		GLint location = glGetUniformLocation(Program, attributeName);
		glUniform1f(location, value);
	}

	void SetglUniformValue(GLchar* attributeName, GLint value){
		GLint location = glGetUniformLocation(Program, attributeName);
		glUniform1i(location, value);
	}

	void SetglUniformValue(GLchar* attributeName, GLuint value){
		GLint location = glGetUniformLocation(Program, attributeName);
		glUniform1ui(location, value);
	}
};