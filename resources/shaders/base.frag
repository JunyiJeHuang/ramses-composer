#version 300 es
precision highp float;

uniform sampler2D u_Tex;
uniform vec4 u_Col;

in vec2 v_uv;

out vec4 FragColor;

void main()
{
	vec4 col = texture(u_Tex, v_uv) * u_Col;
	FragColor = col;
}
