#version 430
in vec2 uv;
out vec4 frag_color;
uniform sampler2D texture;

void main() {
	//frag_color = vec4(1.0,1.0,0.0,1.0);
	frag_color = texture2D(texture, uv);
}