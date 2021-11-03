#version 430

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 texcoord;

uniform mat4 Projection;
uniform mat4 ModelView;

out vec2 uv;

void main() {
	gl_Position = Projection * ModelView * vec4(in_position, 1.0);
	uv = texcoord;
}