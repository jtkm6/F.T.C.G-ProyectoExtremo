// STD. Includes
#include <list>
#include <vector>
#include <string>

// Windows include
#define NOMINMAX
#include <Windows.h>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GL includes
#include <TransferFunction.hpp>
#include <Shader.hpp>
#include <Volume.hpp>

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Other Libs
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// Properties
GLuint screenWidth = 800, screenHeight = 600;
int tf_screenWidth = 360, tf_screenHeight = 455;

// Execution vars
glm::mat4 PV;
GLboolean light = false;

GLFWwindow *volume, *transferFunction;

TransferFunction *transfer_function;
double mouse_xpos_tf, mouse_ypos_tf;


GLboolean mouse_movement_flag = false;
GLfloat mouse_xpos, mouse_ypos, clik_mouse_xpos, clik_mouse_ypos;

std::vector<bool> keys(1024, false);

std::list<Volume*> volumes;
std::list<Volume*>::iterator selected_volume;

void error_callback(int error, const char* description){
	OutputDebugStringA(description);
}

// GLFW volume callback

void volume_window_size_callback(GLFWwindow* window, int width, int height){
	glfwMakeContextCurrent(volume);
	screenWidth = width; screenHeight = height;
	glViewport(0, 0, screenWidth, screenHeight);
	// Eye configuration
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)screenWidth / screenHeight, 0.1f, 100.0f);
	glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 4.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 PV = projection * view;
	// Volume update
	for(auto it = volumes.begin(); it != volumes.end(); ++it)
		(*it)->ResizeScreen(glm::vec2(screenWidth, screenHeight));
}

void volume_key_callback(GLFWwindow* window, int key, int scancode, int action, int mode){
	glfwMakeContextCurrent(volume);
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if(key == GLFW_KEY_RIGHT && action == GLFW_PRESS){
		selected_volume++;
		if(selected_volume == volumes.end())
			--selected_volume;
	}
	if(key == GLFW_KEY_LEFT && action == GLFW_PRESS){
		if(selected_volume != volumes.begin())
			--selected_volume;
	}
	if(key == GLFW_KEY_L && action == GLFW_PRESS){
		for(auto it = volumes.begin(); it != volumes.end(); ++it)
			(*it)->SetLight();
		light = !light;
	}
	if(action == GLFW_PRESS)
		keys[key] = true;
	else if(action == GLFW_RELEASE)
		keys[key] = false;
}

void volume_mouse_callback(GLFWwindow* window, double xpos, double ypos){
	glfwMakeContextCurrent(volume);
	mouse_xpos = (GLfloat)xpos;
	mouse_ypos = (GLfloat)ypos;
	if(mouse_movement_flag){
		glm::vec2 actual = (*selected_volume)->GetRotation();
		GLfloat offset_x = (mouse_xpos - clik_mouse_xpos) * 0.001f;
		GLfloat offset_y = (mouse_ypos - clik_mouse_ypos) * 0.001f;
		(*selected_volume)->SetRotation(actual.x + offset_y, actual.y + offset_x);
	}
	if(light){
		float x = ((float)xpos - screenWidth) / screenWidth;
		float y = ((float)ypos - screenHeight) / screenHeight;
		for(auto it = volumes.begin(); it != volumes.end(); ++it)
			(*it)->SetLightPosition(glm::vec3(-x, y, -4.f));
	}
}

void volume_mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
	glfwMakeContextCurrent(volume);
	float nuevo = (*selected_volume)->GetEscalation() + (float)(yoffset * 0.01);
	if(nuevo > 0.5f && nuevo < 1.8f)
		(*selected_volume)->SetEscalation(nuevo);
}

void volume_mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
	glfwMakeContextCurrent(volume);
	if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
		mouse_movement_flag = true;
		clik_mouse_xpos = mouse_xpos; clik_mouse_ypos = mouse_ypos;
	} else if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE){
		mouse_movement_flag = false;
		(*selected_volume)->SaveRotation();
	}
}

void transfer_function_mouse_callback(GLFWwindow* window, double xpos, double ypos){
	glfwMakeContextCurrent(transferFunction);
	mouse_xpos_tf = xpos;
	mouse_ypos_tf = ypos;
	transfer_function->CursorPos((int)xpos, (int)ypos);
}

void transfer_function_mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
	glfwMakeContextCurrent(transferFunction);
	transfer_function->MouseButton((int)mouse_xpos_tf, (int)mouse_ypos_tf, button, action);
}

void do_action(){
	glfwMakeContextCurrent(volume);
	static const float mov_factor = 0.01f;
	if(keys[GLFW_KEY_UP]){
		glm::vec3 aux = (*selected_volume)->GetTranslation();
		(*selected_volume)->SetTranslation(glm::vec3(aux.x, aux.y + mov_factor, aux.z));
	}else if(keys[GLFW_KEY_DOWN]){
		glm::vec3 aux = (*selected_volume)->GetTranslation();
		(*selected_volume)->SetTranslation(glm::vec3(aux.x, aux.y - mov_factor, aux.z));
	}else if(keys[GLFW_KEY_A]){
		glm::vec3 aux = (*selected_volume)->GetTranslation();
		(*selected_volume)->SetTranslation(glm::vec3(aux.x - mov_factor, aux.y, aux.z));
	}else if(keys[GLFW_KEY_D]){
		glm::vec3 aux = (*selected_volume)->GetTranslation();
		(*selected_volume)->SetTranslation(glm::vec3(aux.x + mov_factor, aux.y, aux.z));
	}else if(keys[GLFW_KEY_W]){
		glm::vec3 aux = (*selected_volume)->GetTranslation();
		(*selected_volume)->SetTranslation(glm::vec3(aux.x, aux.y, aux.z - mov_factor));
	}else if(keys[GLFW_KEY_S]){
		glm::vec3 aux = (*selected_volume)->GetTranslation();
		(*selected_volume)->SetTranslation(glm::vec3(aux.x, aux.y, aux.z + mov_factor));
	}
	if(transfer_function->NeedUpdate()){
		GLfloat **function = transfer_function->GetFunction();
		for(auto it = volumes.begin(); it != volumes.end(); ++it)
			(*it)->UpdateTransferFunction(function);
	}
}

// The MAIN function, from here we start our application and run our Game loop
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow){
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	glfwSetErrorCallback(error_callback);

	if(!glfwInit()){
		OutputDebugStringA("ERROR::MAIN::glfwInit()\n");
		return EXIT_FAILURE;
	}
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	volume = glfwCreateWindow(screenWidth, screenHeight, "F.T.C.G. Volume Rendering | Jorge Khabazze | 23.692.079", nullptr, nullptr);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	transferFunction = glfwCreateWindow(tf_screenWidth, tf_screenHeight, "F.T.C.G. Transfer Function", nullptr, nullptr);
	if(!volume || !transferFunction){
		OutputDebugStringA("ERROR::MAIN::glfwCreateWindow(...)\n");
		glfwTerminate();
		return EXIT_FAILURE;
	}
	// GLFW callback
	glfwSetMouseButtonCallback(volume, volume_mouse_button_callback);
	glfwSetWindowSizeCallback(volume, volume_window_size_callback);
	glfwSetScrollCallback(volume, volume_mouse_scroll_callback);
	glfwSetCursorPosCallback(volume, volume_mouse_callback);
	glfwSetKeyCallback(volume, volume_key_callback);

	glfwSetMouseButtonCallback(transferFunction, transfer_function_mouse_button_callback);
	glfwSetCursorPosCallback(transferFunction, transfer_function_mouse_callback);

	////////// config transfer function context //////////
	glfwMakeContextCurrent(transferFunction);

	// Initialize GLEW to setup the OpenGL Function pointers
	glewExperimental = GL_TRUE;
	glewInit();

	// Define the viewport dimensions
	glViewport(0, 0, tf_screenWidth, tf_screenHeight);
	Shader tis("./shaders/TransferFunction.vert", "./shaders/TransferFunction.frag");
	transfer_function = new TransferFunction(tis);

	////////// config volume context //////////
	glfwMakeContextCurrent(volume);

	// Initialize GLEW to setup the OpenGL Function pointers
	glewExperimental = GL_TRUE;
	glewInit();

	// Define the viewport dimensions
	glViewport(0, 0, screenWidth, screenHeight);

	// Load the shaders
	Shader backface("./shaders/backface.vs", "./shaders/backface.fs");
	Shader raycasting("./shaders/raycasting.vs", "./shaders/raycasting.fs");
	Shader raycasting_plus_light("./shaders/raycasting.vs", "./shaders/raycasting_plus_light.fs");

	// Load the volume
	volumes.push_back(new Volume("./volume/bonsai.raw", glm::vec3(256u, 256u, 256u), 8u, glm::vec2(screenWidth, screenHeight), backface, raycasting, raycasting_plus_light));
	volumes.back()->SetTranslation(glm::vec3(-1.f, 0.f, 0.f));
	volumes.push_back(new Volume("./volume/vertebra.raw", glm::vec3(512u, 512u, 512u), 16u, glm::vec2(screenWidth, screenHeight), backface, raycasting, raycasting_plus_light));
	volumes.back()->SetTranslation(glm::vec3(1.f, 0.f, 0.f));
	selected_volume = volumes.begin();

	// Eye configuration
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)screenWidth / screenHeight, 0.1f, 100.0f);
	glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 4.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 PV = projection * view;

	// Keep the program runing
	while(!glfwWindowShouldClose(volume) && !glfwWindowShouldClose(transferFunction)){
		// Volume window render
		glfwMakeContextCurrent(volume);
		// Clear the colorbuffer
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Render the volume
		for(auto it = volumes.begin(); it != volumes.end(); ++it)
			(*it)->Render(PV);

		// Transfer function window render
		glfwMakeContextCurrent(transferFunction);
		// Clear the colorbuffer
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		transfer_function->Render();
		check_gl_error();
		// Swap the buffers
		glfwSwapBuffers(volume);
		glfwSwapBuffers(transferFunction);
		glfwPollEvents();
		do_action();
	}

	glfwDestroyWindow(transferFunction);
	glfwDestroyWindow(volume);
	glfwTerminate();
	return 0;
}
