#version 300 es
precision highp float;

uniform mat4 u_MVPMatrix;

in vec3 a_Position;
in vec2 a_TextureCoordinate;

out vec2 v_uv;

void main() 
{
	v_uv = a_TextureCoordinate;
	gl_Position = u_MVPMatrix*vec4(a_Position, 1.0);
}
