#pragma once

// STD. Includes
#include <queue>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

// GL includes
#include <Shader.hpp>

// GLEW
#include <GL/glew.h>

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Other Libs
#include <stb_image.h>

void _check_gl_error(const char *file, int line){
	GLenum err(glGetError());

	while(err != GL_NO_ERROR){
		std::string error;

		switch(err){
			case GL_INVALID_OPERATION:      error = "INVALID_OPERATION";      break;
			case GL_INVALID_ENUM:           error = "INVALID_ENUM";           break;
			case GL_INVALID_VALUE:          error = "INVALID_VALUE";          break;
			case GL_OUT_OF_MEMORY:          error = "OUT_OF_MEMORY";          break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:  error = "INVALID_FRAMEBUFFER_OPERATION";  break;
		}
		char arr[20148];
		sprintf_s(arr, "GL_%s - %s:%d\n", error.c_str(), file, line);
		OutputDebugStringA(arr);
		err = glGetError();
	}
}
#define check_gl_error() _check_gl_error(__FILE__,__LINE__)

#define BUFFER_OFFSET(i) (reinterpret_cast<void*>(i))

#define NUMOFCOLORS 256

#define IMAGEWIDHT 360
#define IMAGEHEIGHT 210
#define MINW 47
#define MINH 26
#define MAXW 328
#define MAXH 155
#define MAXPOINT 100

#define MINWSC 0
#define MINHSC 212
#define MAXWSC 355
#define MAXHSC 232

#define MINWPC 0
#define MINHPC 241
#define MAXWPC 360
#define MAXHPC 448

#define SIZEW 360
#define SIZEH 455

struct HSV{
	float h;
	float s;
	float v;
};

struct ControlPoint{
	HSV hsv;
	glm::vec4 color;
	int scalar;
	bool drag, selected;
};

bool operator<(const ControlPoint &a, const ControlPoint &b){
	if(a.scalar > b.scalar)
		return true;
	if(a.scalar == b.scalar)
		if(a.color.a < b.color.a)
			return true;
	return false;
}

class TransferFunction{
protected:
	int control_points;
	int last_picking, point_selected;
	GLuint texture_graphic, texture_point, texture_selector, quad_VBO[2], quad_VAO;
	bool drag_drop_flag, drag_drop_color_flag, drag_drop_picker_flag, update_data_flag;
	ControlPoint control_point[MAXPOINT];
	glm::vec4 base_color[6], current_color;
	glm::mat4x4 P, M;
	glm::ivec2 post_transfer_function, pos_HUE, pos_SV;
	Shader shader;
	GLfloat **color_palette;

	GLuint load_texture2D(const char* filename){
		int width, height, comp;
		GLuint gl_texID;
		stbi_set_flip_vertically_on_load(true);
		unsigned char* bits = stbi_load(filename, &width, &height, &comp, 0);
		glGenTextures(1, &gl_texID);
		glBindTexture(GL_TEXTURE_2D, gl_texID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bits);
		stbi_image_free(bits);
		return gl_texID;
	}

	void init_quad(){
		GLfloat Vertex[] = {
			-0.5f, -0.5f,	0.0f, 0.0f,
			 0.5f, -0.5f,	1.0f, 0.0f,
			 0.5f,  0.5f,	1.0f, 1.0f,
			-0.5f,  0.5f,	0.0f, 1.0f
		};

		GLuint Indices[] = {0, 1, 2, 0, 2, 3};

		glGenBuffers(2, quad_VBO);
		glBindBuffer(GL_ARRAY_BUFFER, quad_VBO[0]);
		glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(GL_FLOAT), Vertex, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_VBO[1]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GL_UNSIGNED_INT), Indices, GL_STATIC_DRAW);
		glGenVertexArrays(1, &quad_VAO);
		glBindVertexArray(quad_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, quad_VBO[0]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_VBO[1]);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 4, BUFFER_OFFSET(0));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 4, BUFFER_OFFSET(sizeof(GL_FLOAT) * 2));
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void draw_quad(){
		glBindVertexArray(quad_VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	void draw_graphic(){
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_graphic);

		glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		M = glm::scale(glm::translate(glm::mat4(), glm::vec3(IMAGEWIDHT * 0.5f, -IMAGEHEIGHT * 0.5f + SIZEH, 0.0f)), glm::vec3(IMAGEWIDHT, IMAGEHEIGHT, 1.0f));

		shader.Enable();
		shader.SetglUniformValue("mProjection", P);
		shader.SetglUniformValue("vColor1", color);
		shader.SetglUniformValue("Usetexture", 1);
		shader.SetglUniformValue("text", 0);
		shader.SetglUniformValue("Mode", 0);
		shader.SetglUniformValue("mModelView", M);

		draw_quad();
		shader.Disable();
		check_gl_error();
	}

	void draw_color_choser(){
		float height = float(SIZEH - (IMAGEHEIGHT + 2));
		float size_widht = (float)IMAGEWIDHT / 6.0f, size_height = 30;

		shader.Enable();
		for(int i = 0; i < 6; ++i){
			M = glm::scale(glm::translate(glm::mat4(), glm::vec3(size_widht * 0.5f + size_widht * i, -size_height * 0.5f + height, 0.0f)), glm::vec3(size_widht, size_height, 1.0f));
			shader.SetglUniformValue("vColor1", base_color[i]);
			shader.SetglUniformValue("vColor2", base_color[(i + 1) % 6]);
			shader.SetglUniformValue("vColor3", base_color[(i + 1) % 6]);
			shader.SetglUniformValue("vColor4", base_color[i]);
			shader.SetglUniformValue("Usetexture", 0);
			shader.SetglUniformValue("Mode", 1);
			shader.SetglUniformValue("mModelView", M);
			draw_quad();
		}
		shader.Disable();
		check_gl_error();
	}

	void draw_selector(){
		float height = float(SIZEH - (IMAGEHEIGHT + 2));
		float size_widht = (float)IMAGEWIDHT / 36.0f, size_height = 30;

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_selector);

		glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		M = glm::scale(glm::translate(glm::mat4(), glm::vec3(size_widht * 0.5f + pos_HUE.x, -size_height * 0.5f + height, 0.0f)), glm::vec3(size_widht, size_height, 1.0f));											//Scale
	
		shader.Enable();
		shader.SetglUniformValue("vColor1", color);
		shader.SetglUniformValue("Usetexture", 1);
		shader.SetglUniformValue("text", 0);
		shader.SetglUniformValue("Mode", 0);
		shader.SetglUniformValue("mModelView", M);
		draw_quad();
		shader.Disable();
		check_gl_error();
	}

	void draw_imag(){
		float height = float(SIZEH - (IMAGEHEIGHT + 34));
		float size_widht = (float)IMAGEWIDHT, size_height = (float)IMAGEHEIGHT;

		M = glm::scale(glm::translate(glm::mat4(), glm::vec3(size_widht * 0.5f, -size_height * 0.5f + height, 0.0f)), glm::vec3(size_widht, size_height, 1.0f));

		shader.Enable();
		shader.SetglUniformValue("vColor1", glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		shader.SetglUniformValue("vColor2", glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		shader.SetglUniformValue("vColor3", current_color);
		shader.SetglUniformValue("vColor4", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		shader.SetglUniformValue("Usetexture", 0);
		shader.SetglUniformValue("Mode", 1);
		shader.SetglUniformValue("mModelView", M);
		draw_quad();
		shader.Disable();
		check_gl_error();
	}

	void draw_graphics(){
		static float graphic_alpha = 0.9f;
		for(int point = 1; point < control_points; point++){
			glm::ivec2 pos_0 = scalar_alpha_to_screen_pos(control_point[point - 1].scalar, control_point[point - 1].color.a);
			glm::ivec2 pos_1 = scalar_alpha_to_screen_pos(control_point[point].scalar, control_point[point].color.a);
	
			float size_widht = (float)abs(pos_1.x - pos_0.x), size_height = (float)abs(pos_1.y - pos_0.y);
			float height = (float)std::min(pos_1.y, pos_0.y), width = (float)std::min(pos_1.x, pos_0.x);

			M = glm::scale(glm::translate(glm::mat4(), glm::vec3(size_widht * 0.5f + width, -size_height * 0.5f + SIZEH - height, 0.0f)), glm::vec3(size_widht, size_height, 1.0f));

			shader.Enable();
			shader.SetglUniformValue("vColor1", glm::vec4(control_point[point - 1].color.x, control_point[point - 1].color.y, control_point[point - 1].color.z, graphic_alpha));
			shader.SetglUniformValue("vColor2", glm::vec4(control_point[point].color.x, control_point[point].color.y, control_point[point].color.z, graphic_alpha));
			shader.SetglUniformValue("vColor3", glm::vec4(control_point[point].color.x, control_point[point].color.y, control_point[point].color.z, graphic_alpha));
			shader.SetglUniformValue("vColor4", glm::vec4(control_point[point - 1].color.x, control_point[point - 1].color.y, control_point[point - 1].color.z, graphic_alpha));
			shader.SetglUniformValue("Usetexture", 0);
			if(pos_0.y > pos_1.y)
				shader.SetglUniformValue("Mode", 2);
			else
				shader.SetglUniformValue("Mode", 3);
			shader.SetglUniformValue("mModelView", M);
			draw_quad();
			shader.Disable();

			size_widht = float(pos_1.x - pos_0.x);
			size_height = float(MAXH - std::max(pos_1.y, pos_0.y));

			if(size_height > 0){
				M = glm::scale(glm::translate(glm::mat4(), glm::vec3(size_widht * 0.5f + width, size_height * 0.5f + SIZEH - MAXH, 0.0f)), glm::vec3(size_widht, size_height, 1.0f));
				shader.Enable();
				shader.SetglUniformValue("Mode", 1);
				shader.SetglUniformValue("mModelView", M);
				draw_quad();
				shader.Disable();
			}
		}
	}

	void draw_points(){
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_point);

		shader.Enable();
		for(int point = 0; point < control_points; point++){
			glm::vec4 color;
			if(point != point_selected - 1)
				color = glm::vec4(1.0f, 1.0f, 1.0f, 0.5f);
			else
				color = glm::vec4(0.0f, 0.8f, 1.0f, 1.0f);

			glm::ivec2 pos = scalar_alpha_to_screen_pos(control_point[point].scalar, control_point[point].color.a);

			M = glm::scale(glm::translate(glm::mat4(), glm::vec3(pos.x, -pos.y + SIZEH, 0.0f)), glm::vec3(10.0f, 10.0f, 1.0f));
			shader.SetglUniformValue("vColor1", color);
			shader.SetglUniformValue("Usetexture", 1);
			shader.SetglUniformValue("Mode", 0);
			shader.SetglUniformValue("mModelView", M);
			draw_quad();
		}
		shader.Disable();
	}

	void draw_circle_selector(){
		glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

		M = glm::scale(glm::translate(glm::mat4(), glm::vec3(pos_SV.x, -5.0f - pos_SV.y + SIZEH, 0.0f)), glm::vec3(10, 10, 1.0f));
		shader.Enable();
		shader.SetglUniformValue("vColor1", color);
		shader.SetglUniformValue("Usetexture", 1);
		shader.SetglUniformValue("Mode", 0);
		shader.SetglUniformValue("mModelView", M);
		draw_quad();
		shader.Disable();
	}

	void delete_point(int w, int h){
		int last_selected_point = point_selected;
		if(picking(w, h)){
			if(last_picking != 1 && last_picking != control_points){
				if(last_selected_point == last_picking)
					point_selected = 0;
				else
					point_selected = last_selected_point;

				sort_points(last_picking - 1);
				update_pallete();
				point_selected = 0;
				last_picking = -1;
				control_points--;
			} else
				point_selected = 0;

			drag_drop_flag = false;
		}
	}

	bool picking(int x, int y){
		//Search for the point
		for(int point = 0; point < control_points; point++){

			glm::ivec2 pos = scalar_alpha_to_screen_pos(control_point[point].scalar, control_point[point].color.a);

			//Take advantage of the point's sorting
			if(x < pos.x - 5) break;

			//The right coordinate
			if(pos.x + 5 > x && pos.x - 5 < x &&
				pos.y + 5 > y && pos.y - 5 < y){
				point_selected = last_picking = point + 1;
				drag_drop_flag = true;
				return true;
			}
		}

		return false;
	}

	void sort_points(int jumpPoint = -1){
		std::priority_queue< ControlPoint > state;
		int less = (jumpPoint == -1) ? 0 : 1;

		//Insert the points in a priority queue
		for(int point = 0; point < control_points; point++){
			if(point != jumpPoint){
				control_point[point].drag = ((drag_drop_flag) && (point == last_picking - 1));
				control_point[point].selected = (point == point_selected - 1);
				state.push(control_point[point]);
			}
		}

		//Put them back into the array
		for(int point = 0; point < control_points - less; point++){

			control_point[point] = state.top();
			if(control_point[point].drag)
				last_picking = point + 1;
			if(control_point[point].selected)
				point_selected = point + 1;

			control_point[point].drag = false;
			control_point[point].selected = false;
			state.pop();

		}
	}

	void update_pallete(){
		int index = 0;

		//interpolation between two scalar values
		for(int point = 1; point < control_points; point++){
			int dist = control_point[point].scalar - control_point[point - 1].scalar;
			int stepsNumber = dist;

			float floatStepSize = 1.0f / float(dist);

			for(int step = 0; step < stepsNumber; step++, index++){
				glm::vec4 currentColor = control_point[point - 1].color * (1.0f - (floatStepSize * (float)step)) + control_point[point].color * (floatStepSize * (float)step);
				color_palette[index][0] = currentColor.x;
				color_palette[index][1] = currentColor.y;
				color_palette[index][2] = currentColor.z;
				color_palette[index][3] = currentColor.w;
			}
		}

		//If there are positions to fill
		if(index < 256){
			glm::vec4 color = control_point[control_points - 1].color;

			for(int step = index; step < 256; step++, index++){
				color_palette[index][0] = color.x;
				color_palette[index][1] = color.y;
				color_palette[index][2] = color.z;
				color_palette[index][3] = color.w;
			}
		}
		update_data_flag = true;
	}

	void update_color_point(){
		// get the scalar and alpha from the screen position
		int scalar = screen_pos_to_scalar();
		float alpha = screen_pos_to_alpha();

		//get the color from the hue, saturation, and value from screen
		glm::vec3 color = screen_pos_to_RGB();

		//set the values to the point
		control_point[point_selected - 1].scalar = scalar;
		control_point[point_selected - 1].color = glm::vec4(color, alpha);
		control_point[point_selected - 1].hsv = RGB_to_HSV(glm::vec3(control_point[point_selected - 1].color));

		//update the current color to draw it properly
		set_current_color();
	}

	void set_current_color(){
		//Set the current color on the interface
		HSV hsv;
		hsv.h = control_point[point_selected - 1].hsv.h;
		if(hsv.h == 0.0f) hsv.h = screen_pos_to_HUE();
		hsv.s = 1.0f;
		hsv.v = 1.0f;
		current_color = glm::vec4(HSV_to_RGB(hsv), 1.0f);
	}

	int screen_pos_to_scalar(){
		return int((post_transfer_function.x - MINW) / (float)(MAXW - MINW) * 255);
	}

	float screen_pos_to_alpha(){
		return 1.0f - ((float)(post_transfer_function.y - MINH) / (float)(MAXH - MINH));
	}

	float screen_pos_to_HUE(){
		return (pos_HUE.x - MINWSC) / float(MAXWSC - MINWSC);
	}

	HSV RGB_to_HSV(glm::vec3 colorRGB){
		float cmax = fmaxf(colorRGB.r, fmaxf(colorRGB.g, colorRGB.b));
		float cmin = fminf(colorRGB.r, fminf(colorRGB.g, colorRGB.b));
		float cdif = cmax - cmin;


		HSV hsv;

		hsv.v = cmax;

		if(cdif < 0.00001f) //undefined
		{
			hsv.s = 0;
			hsv.h = 0; // undefined, maybe nan?
			return hsv;
		}


		hsv.h = 0;

		if(fabsf(cdif) <  0.001f) 0;
		else if(fabsf(cmax - colorRGB.r) < 0.001f) hsv.h = fmod((colorRGB.g - colorRGB.b) / cdif, 6.0f);
		else if(fabsf(cmax - colorRGB.g) < 0.001f)	hsv.h = 2.0f + (colorRGB.b - colorRGB.r) / cdif;
		else if(fabsf(cmax - colorRGB.b) < 0.001f)	hsv.h = 4.0f + (colorRGB.r - colorRGB.g) / cdif;

		hsv.h = (hsv.h * 60.0f);

		hsv.h = ((hsv.h < 0.0f) ? (360.0f + hsv.h) : hsv.h) / 360.0f;

		hsv.s = 0;
		if(fabsf(cmax) < 0.001f) hsv.s = 0;
		else hsv.s = cdif / cmax;



		return hsv;
	}

	glm::vec3 HSV_to_RGB(HSV hsv){
		float c = hsv.v * hsv.s;
		float hPrime = hsv.h * 360.0f / 60.0f;
		float x = c * (1.0f - fabsf(fmodf(hPrime, 2) - 1.0f));
		float m = hsv.v - c;
		glm::vec3 rgb;

		if(hPrime < 1){
			rgb = glm::vec3(c, x, 0.0f);
		} else if(hPrime< 2){
			rgb = glm::vec3(x, c, 0.0f);
		} else if(hPrime < 3){
			rgb = glm::vec3(0.0f, c, x);
		} else if(hPrime < 4){
			rgb = glm::vec3(0.0f, x, c);
		} else if(hPrime < 5){
			rgb = glm::vec3(x, 0.0f, c);
		} else if(hPrime < 6){
			rgb = glm::vec3(c, 0.0f, x);
		}

		return glm::vec3(rgb.r + m, rgb.g + m, rgb.b + m);
	}

	glm::ivec2 scalar_alpha_to_screen_pos(int scalar, float alpha){
		glm::ivec2 pos;

		pos.x = int((scalar / 255.0f) * (MAXW - MINW) + MINW);
		pos.y = int((1.0f - alpha) * (MAXH - MINH) + MINH);

		return pos;
	}

	glm::vec3 screen_pos_to_RGB(){
		float ts = (float)pos_SV.x / (float)MAXWPC;
		float tv = 1.0f - (float)(pos_SV.y - MINHPC) / (float)(MAXHPC - MINHPC);

		HSV hsv;
		//get hue
		hsv.h = (pos_HUE.x - MINWSC) / float(MAXWSC - MINWSC);
		hsv.s = ts;
		hsv.v = tv;

		//transform it to RGB
		glm::vec3 final = HSV_to_RGB(hsv);

		return final;
	}

	glm::ivec2 HSV_to_screen_pos_HUE(HSV hsv){
		glm::ivec2 pos;

		pos.x = int(hsv.h*(MAXWSC - MINWSC) + MINWSC);
		pos.y = int(0 * (MAXHSC - MINHSC) + MINHSC);

		return pos;
	}

	glm::ivec2 HSV_to_screen_pos_SV(HSV hsv){
		glm::ivec2 pos;

		pos.x = int(hsv.s*(MAXWPC - MINWPC) + MINWPC);
		pos.y = int((1.0f - hsv.v)*(MAXHPC - MINHPC) + MINHPC);
		return pos;
	}

public:
	TransferFunction(Shader &program) : control_points(0), last_picking(0),
		drag_drop_flag(false), drag_drop_color_flag(false), drag_drop_picker_flag(false), point_selected(1),
		update_data_flag(true), shader(program){
		base_color[0] = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		base_color[1] = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
		base_color[2] = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
		base_color[3] = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
		base_color[4] = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
		base_color[5] = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
		current_color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

		pos_HUE = glm::ivec2(MINWPC, MINHPC + 6);
		pos_SV = glm::ivec2(MINWPC + 5, MINHPC + 5);

		glActiveTexture(GL_TEXTURE0);
		texture_graphic = load_texture2D("./images/transferFunction.png");
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		texture_point = load_texture2D("./images/point.png");
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		texture_selector = load_texture2D("./images/selector.png");
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		init_quad();

		color_palette = new GLfloat*[256];
		for(size_t i = 0; i < 256; ++i){
			color_palette[i] = new GLfloat[4];
			color_palette[i][0] = color_palette[i][1] = color_palette[i][2] = color_palette[i][3] = 0.f;
		}

		check_gl_error();
	}

	void Render(){
		glViewport(0, 0, SIZEW, SIZEH);
		P = glm::ortho(0.0f, (float)SIZEW, 0.0f, (float)SIZEH);

		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		draw_graphic();
		draw_color_choser();
		draw_selector();
		draw_imag();
		draw_graphics();
		draw_points();
		draw_circle_selector();

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		
	}

	void MouseButton(int w, int h, int button, int action){
		if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
			// Graphic area
			if(w >= MINW - 5 && w <= MAXW + 5 && h >= MINH - 5 && h <= MAXH + 5){
				// If you dont pick a control point
				if(!picking(w, h)){
					// Add a point if you can
					if(control_points < MAXPOINT - 1){
						if(h < MINH) h = MINH;
						if(h > MAXH) h = MAXH;
						if(w < MINW) w = MINW;
						if(w > MAXW) w = MAXW;

						post_transfer_function.x = w;
						post_transfer_function.y = h;
						point_selected = ++control_points;
						update_color_point();
						sort_points();

						// Allow this point to drag and drop
						picking(w, h);
					}
				}else{
					// If we pick a point update the color chooser and the color selector
					post_transfer_function = scalar_alpha_to_screen_pos(control_point[point_selected - 1].scalar, control_point[point_selected - 1].color.a);
					pos_HUE = HSV_to_screen_pos_HUE(control_point[point_selected - 1].hsv);
					pos_SV = HSV_to_screen_pos_SV(control_point[point_selected - 1].hsv);
					set_current_color();
				}
				//Either way update the pallete
				update_pallete();
				return;
			}else if(w >= MINWSC && w <= MAXWSC && h >= MINHSC && h <= MAXHSC){
				//Allways allow picking the color selector
				drag_drop_color_flag = true;
				//Move to the position of the mouse
				pos_HUE.x = w;
				//Update the pallete!!!
				if(point_selected != 0) update_color_point();
				update_pallete();
				return;
			}else if(w >= MINWPC && w <= MAXWPC && h >= MINHPC && h <= MAXHPC){
				//Allways allow picking the color picker
				drag_drop_picker_flag = true;
				//Clamp coordinates
				h = h - 5;
				if(w < MINWPC + 5)
					w = MINWPC + 5;
				if(w > MAXWPC - 5)
					w = MAXWPC - 5;
				if(h < MINHPC + 5)
					h = MINHPC + 5;
				if(h > MAXHPC - 5)
					h = MAXHPC - 5;
				//Change its position
				pos_SV.x = w;
				pos_SV.y = h;
				//Update the pallete!!!
				if(point_selected != 0) update_color_point();
				update_pallete();
				return;
			}
		}else if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS){
			delete_point(w, h);
		}else if(action == GLFW_RELEASE){
			//Reset all te drag and drops
			drag_drop_flag = false;
			drag_drop_color_flag = false;
			drag_drop_picker_flag = false;
		}
	}

	void CursorPos(int w, int h){
		static int DIVBASESC = MAXWSC / 6;
		if(drag_drop_flag){
			//Drag and drop the points
			if(w > MAXW)
				w = MAXW;
			if(h > MAXH)
				h = MAXH;
			if(w < MINW)
				w = MINW;
			if(h < MINH)
				h = MINH;

			if(last_picking - 1 != 0 && last_picking != control_points)
				post_transfer_function.x = w;
			post_transfer_function.y = h;

			if(point_selected != 0)
				update_color_point();
			sort_points();
			update_pallete();
		}else if(drag_drop_color_flag){
			if(w > MAXWSC - 5)
				w = MAXWSC - 5;
			if(w < MINWSC)
				w = MINWSC;

			//Allways allow picking the color selector
			drag_drop_color_flag = true;

			//Move to the position of the mouse
			pos_HUE.x = w;

			//Update the point!!!
			if(point_selected != 0) update_color_point();
			update_pallete();

		}else if(drag_drop_picker_flag){
			//Allways allow picking the color picker
			h = h - 5;
			if(w < MINWPC + 5) w = MINWPC + 5; if(w > MAXWPC - 5) w = MAXWPC - 5;
			if(h < MINHPC + 5) h = MINHPC + 5; if(h > MAXHPC - 5) h = MAXHPC - 5;

			//Change its position
			pos_SV.x = w;
			pos_SV.y = h;

			//Update the selected Point!!!
			if(point_selected != 0)
				update_color_point();
			update_pallete();
		}
	}

	GLfloat **GetFunction(){
		update_data_flag = false;
		return color_palette;
	}

	bool NeedUpdate(){
		return update_data_flag;
	}

};