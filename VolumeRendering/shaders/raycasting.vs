#version 330

// In values.
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 volume_position;
uniform mat4 MVP;

// Out values.
out vec3 in_coordinate;

void main(){
	in_coordinate = volume_position;
    gl_Position = MVP * vec4(vertex_position, 1.0);
}
