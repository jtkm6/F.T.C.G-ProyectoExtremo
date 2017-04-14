#pragma once

// STD. Includes
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

// GL includes
#include <Shader.hpp>

// GLEW
#include <GL/glew.h>

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define BUFFER_OFFSET(i) (reinterpret_cast<void*>(i))

class Volume{
private:
	Shader backface, raycasting, raycasting_plus_light;
	GLboolean light = false;
	GLuint screen_width, screen_height, bits;
	GLuint object_buffer[2], array_buffer, depth_buffer, frame_buffer;
	GLuint transfer_function_texture = -1, backface_texture, volume_texture;
	glm::vec3 escalation, translation, voxel_size, light_position, light_color;
	glm::quat rotation; GLfloat rot_x = 0, aux_rot_x, rot_y = 0, aux_rot_y;

	void init_vob(){

		static const GLfloat vertices[] = {
			-1.0f,-1.0f, 1.0f,		0.0f,0.0f,0.0f,	//v0
			 1.0f,-1.0f, 1.0f,		1.0f,0.0f,0.0f,	//v1
			 1.0f, 1.0f, 1.0f,		1.0f,1.0f,0.0f,	//v2
			-1.0f, 1.0f, 1.0f,		0.0f,1.0f,0.0f,	//v3
			-1.0f,-1.0f,-1.0f,		0.0f,0.0f,1.0f,	//v4
			 1.0f,-1.0f,-1.0f,		1.0f,0.0f,1.0f,	//v5
			 1.0f, 1.0f,-1.0f,		1.0f,1.0f,1.0f,	//v6
			-1.0f, 1.0f,-1.0f,		0.0f,1.0f,1.0f,	//v7
		};

		static const GLuint indices[] = {
			0,1,2,0,2,3,	//front
			4,7,6,4,6,5,	//back
			4,0,3,4,3,7,	//left
			1,5,6,1,6,2,	//right
			3,2,6,3,6,7,	//top
			4,5,1,4,1,0,	//bottom
		};

		glGenBuffers(2, object_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, object_buffer[0]);
		glBufferData(GL_ARRAY_BUFFER, 48 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object_buffer[1]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(GLuint), indices, GL_STATIC_DRAW);

		glGenVertexArrays(1, &array_buffer);
		glBindVertexArray(array_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, object_buffer[0]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object_buffer[1]);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 6, BUFFER_OFFSET(0));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 6, BUFFER_OFFSET(sizeof(GL_FLOAT) * 3));
		glBindVertexArray(0);
	}

	void load_transfer_function(std::vector<unsigned char> data){
		if(transfer_function_texture == -1)
			glGenTextures(1, &transfer_function_texture);
		glBindTexture(GL_TEXTURE_1D, transfer_function_texture);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA8, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
		glBindTexture(GL_TEXTURE_1D, 0);
	}

	void load_backface(){
		glGenTextures(1, &backface_texture);
		glBindTexture(GL_TEXTURE_2D, backface_texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screen_width, screen_height, 0, GL_RGBA, GL_FLOAT, NULL);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void load_8bits_volume(const std::string patch, const GLuint width, const GLuint height, const GLuint depth){
		std::ifstream file(patch, std::ios::binary);
		if(!file.is_open()){
			OutputDebugStringA("ERROR::VOLUME::FILE_NOT_SUCCESFULLY_READ\n");
			return;
		}
		file.seekg(0, file.end);
		size_t length = static_cast<size_t>(file.tellg());
		if(length != width * height * depth){
			OutputDebugStringA("ERROR::VOLUME::FILE_NOT_SIZE_MATCH\n");
			return;
		}
		file.seekg(0, file.beg);
		char *texture = new char[length];
		file.read((char*)texture, length);
		file.close();
		glGenTextures(1, &volume_texture);
		glBindTexture(GL_TEXTURE_3D, volume_texture);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_INTENSITY, width, height, depth, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, texture);
		glBindTexture(GL_TEXTURE_3D, 0);
		delete[] texture;
	}

	void load_16bits_volume(const std::string patch, const GLuint width, const GLuint height, const GLuint depth){
		std::ifstream file(patch, std::ios::binary);
		if(!file.is_open()){
			OutputDebugStringA("ERROR::VOLUME::FILE_NOT_SUCCESFULLY_READ\n");
			return;
		}
		file.seekg(0, file.end);
		size_t length = static_cast<size_t>(file.tellg());
		if(length != width * height * depth * 2u){
			OutputDebugStringA("ERROR::VOLUME::FILE_NOT_SIZE_MATCH\n");
			return;
		}
		file.seekg(0, file.beg);
		short *texture = new short[length];
		file.read((char*)texture, length);
		file.close();
		glGenTextures(1, &volume_texture);
		glBindTexture(GL_TEXTURE_3D, volume_texture);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_INTENSITY, width, height, depth, 0, GL_LUMINANCE, GL_UNSIGNED_SHORT, texture);
		glBindTexture(GL_TEXTURE_3D, 0);
		delete[] texture;
	}

	void init_frame_buffer(){
		glGenRenderbuffers(1, &depth_buffer);
		glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screen_width, screen_height);
		glGenFramebuffers(1, &frame_buffer);
		glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, backface_texture, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);
		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
			OutputDebugStringA("ERROR::VOLUME::GL_FRAMEBUFFER\n");
			return;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void draw_vob(){
		glBindVertexArray(array_buffer);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

public:
	~Volume(){
		glDeleteTextures(1, &transfer_function_texture);
		glDeleteTextures(1, &backface_texture);
		glDeleteTextures(1, &volume_texture);
		glDeleteRenderbuffers(1, &depth_buffer);
		glDeleteFramebuffers(1, &frame_buffer);
		glDeleteVertexArrays(1, &array_buffer);
		glDeleteBuffers(2, object_buffer);
	}

	Volume(const std::string patch, const glm::vec3 volume_dim, const GLuint bits, const glm::vec2 screen_dim,
		Shader &backface, Shader &raycasting, Shader &raycasting_plus_light) : backface(backface), raycasting(raycasting),
		raycasting_plus_light(raycasting_plus_light), escalation(1.0f){
		screen_width = (GLint)screen_dim.x; screen_height = (GLint)screen_dim.y;
		init_vob();
		std::vector<unsigned char> tf(1024);
		int val = 0;
		for(size_t i = 0; i < 1024; i += 4){
			tf[i + 0] = static_cast<unsigned char>(val);
			tf[i + 1] = static_cast<unsigned char>(val);
			tf[i + 2] = static_cast<unsigned char>(val);
			tf[i + 3] = static_cast<unsigned char>(val);
			++val;
		}
		load_transfer_function(tf);
		load_backface();
		if(bits == 8u)
			load_8bits_volume(patch, (GLuint)volume_dim.x, (GLuint)volume_dim.y, (GLuint)volume_dim.z);
		else if(bits == 16u)
			load_16bits_volume(patch, (GLuint)volume_dim.x, (GLuint)volume_dim.y, (GLuint)volume_dim.z);
		this->bits = bits;
		init_frame_buffer();
		// Light parameters
		voxel_size = glm::vec3(1.0f / volume_dim.x, 1.0f / volume_dim.y, 1.0f / volume_dim.z);
		light_position = glm::vec3(0.f, 0.f, -4.0f);
		light_color = glm::vec3(1.f, 1.f, 1.f);
	}

	void Render(glm::mat4 &PV){
		glm::mat4 M = glm::translate(glm::mat4(), translation) * glm::mat4_cast(rotation) * glm::scale(glm::mat4(), escalation);
		glm::mat4 MVP = PV * M;
		////////////// draw intersection cube //////////////
		// Set OpenGL config
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frame_buffer);
		glViewport(0, 0, screen_width, screen_height);
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glFrontFace(GL_CCW);
		glCullFace(GL_FRONT);
		glClearColor(0.0f, 0.0f, 0.0f, 0.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Draw the cube
		backface.Enable();
		backface.SetglUniformValue("MVP", MVP);
		draw_vob();
		backface.Disable();
		// Restore OpenGL config
		glCullFace(GL_BACK);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		////////////// draw volume cube //////////////
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		// Draw the volume
		if(light){
			raycasting_plus_light.Enable();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_1D, transfer_function_texture);
			raycasting_plus_light.SetglUniformValue("transfer_function", 0);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, backface_texture);
			raycasting_plus_light.SetglUniformValue("backface", 1);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_3D, volume_texture);
			raycasting_plus_light.SetglUniformValue("volume", 2);
			// Light parameters
			glm::vec3 light_direction = glm::vec3(normalize(glm::inverse(M) * glm::vec4(light_position, 0.f)));
			raycasting_plus_light.SetglUniformValue("light_direction", light_direction);
			raycasting_plus_light.SetglUniformValue("light_color", light_color);
			raycasting_plus_light.SetglUniformValue("voxel_size", voxel_size);
			raycasting_plus_light.SetglUniformValue("screen_size", glm::vec2((GLfloat)screen_width, (GLfloat)screen_height));
			raycasting_plus_light.SetglUniformValue("MVP", MVP);
			draw_vob();
			raycasting_plus_light.Disable();
		}else{
			raycasting.Enable();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_1D, transfer_function_texture);
			raycasting.SetglUniformValue("transfer_function", 0);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, backface_texture);
			raycasting.SetglUniformValue("backface", 1);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_3D, volume_texture);
			raycasting.SetglUniformValue("volume", 2);
			// Light parameters
			raycasting.SetglUniformValue("screen_size", glm::vec2((GLfloat)screen_width, (GLfloat)screen_height));
			raycasting.SetglUniformValue("MVP", MVP);
			draw_vob();
			raycasting.Disable();
		}
		// Restore OpenGL config
		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
	}

	void SetRotation(const GLfloat rot_x, const GLfloat rot_y){
		this->aux_rot_x = rot_x; this->aux_rot_y = rot_y;
		this->rotation = glm::quat(glm::eulerAngleXY(rot_x, rot_y));
	}

	void SaveRotation(){
		rot_x = aux_rot_x; rot_y = aux_rot_y;
	}

	void SetTranslation(glm::vec3 &translation){
		this->translation = translation;
	}

	void SetEscalation(const GLfloat escalation){
		this->escalation = glm::vec3(escalation);
	}

	void ResizeScreen(const glm::vec2 screen_dim){
		screen_width = (GLint)screen_dim.x; screen_height = (GLint)screen_dim.y;
		glDeleteTextures(1, &backface_texture);
		load_backface();
		glDeleteRenderbuffers(1, &depth_buffer);
		glDeleteFramebuffers(1, &frame_buffer);
		init_frame_buffer();
	}

	float GetEscalation(){
		return escalation.x;
	}

	glm::vec2 GetRotation(){
		return glm::vec2(rot_x, rot_y);
	}

	glm::vec3 GetTranslation(){
		return translation;
	}

	void SetLight(){
		light = !light;
	}

	void SetLightPosition(glm::vec3 &light_position){
		this->light_position = light_position;
	}

	void UpdateTransferFunction(GLfloat **transfer_function){
		std::vector<unsigned char> tf(1024);
		int val = 0;
		for(size_t i = 0; i < 1024; i += 4){
			tf[i + 0] = static_cast<unsigned char>(transfer_function[val][0] * 255);
			tf[i + 1] = static_cast<unsigned char>(transfer_function[val][1] * 255);
			tf[i + 2] = static_cast<unsigned char>(transfer_function[val][2] * 255);
			tf[i + 3] = static_cast<unsigned char>(transfer_function[val][3] * 255);
			++val;
		}
		load_transfer_function(tf);
	}
};
