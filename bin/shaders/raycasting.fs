#version 330

// In values.
uniform sampler1D transfer_function;
uniform sampler2D backface;
uniform sampler3D volume;
uniform vec2 screen_size;
in vec3 in_coordinate;

// Out pixel data.
out vec4 pixel;

const float step_size = 0.01f;

void main(){
	vec3 out_coordinate = texture(backface, gl_FragCoord.xy/screen_size.xy).xyz;

	if(out_coordinate == in_coordinate){
		pixel = vec4(0.f, 0.f, 0.f, 0.f);
		return;
	}

	// Get direction of the ray
	vec3 direction = out_coordinate - in_coordinate;
	float lenght_in_out = length(direction);
	vec3 ray_step = normalize(direction) * step_size;

	// Raycasting color init
	vec4 color = vec4(0.f);
	vec4 bg_color = vec4(0.f);

	// Execution vars
	vec3 actual_coordinate = in_coordinate;
	float lenght_accumulate = 0.f;

	for(float i = 0; i < 1600; ++i){
		// Sample in the scalar field and the transfer function
		vec4 sample = texture(transfer_function, texture(volume, actual_coordinate).x);

		if(sample.a > 0.f){
    	    sample.a = 1.f - pow(1.f - sample.a, step_size * 200.f);
    	    color.rgb += (1.f - color.a) * sample.rgb * sample.a;
    	    color.a += (1.f - color.a) * sample.a;
    	}

		// Increment vars
		actual_coordinate += ray_step;
		lenght_accumulate += step_size;

		// Terminate raycasting
		if(lenght_accumulate >= lenght_in_out){
			color.rgb = color.rgb * color.a + (1.f - color.a) * bg_color.rgb;		
    	    break;
		}else if(color.a > 1.f){
			color.a = 1.f;
			break;
		}
	}

	pixel = color;
}
