#version 330


uniform int Usetexture;
uniform sampler2D tex;

in vec2 vVertexTexture;
in vec4 vVertexColor;

layout(location = 0) out vec4 vFragColor;

void main(void)
{
	if(Usetexture == 1){
		vFragColor = texture(tex, vVertexTexture);
	}else{
		vFragColor = vVertexColor;
	}
}