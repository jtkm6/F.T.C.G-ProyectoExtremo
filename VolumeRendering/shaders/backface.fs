#version 330

// In values.
in vec3 coordinate;

// Out pixel data.
out vec4 pixel;

void main(){
    pixel = vec4(coordinate, 1.0);
}
