#version 300 es
precision highp float;

uniform mat4 u_MVPMatrix;
uniform mat4 u_MMatrix;


in vec3 a_Position;
out vec3 v_WorldPosition;

void main()
{
	v_WorldPosition = vec4(u_MMatrix * vec4(a_Position, 1.0)).xyz;
	
	gl_Position = u_MVPMatrix*vec4(a_Position.xyz, 1.0);
}
