#version 300 es
precision highp float;

uniform mat4 u_MMatrix;
uniform mat4 u_MVPMatrix;

in vec3 a_Position;
in vec2 a_TextureCoordinate;
in vec3 a_Normal;
in vec3 a_Tangent;

out vec2 v_TextureCoordinate;
out vec3 v_WorldPosition;
out vec3 v_WorldNormal;
out vec3 v_WorldTangent;



void main()
{
	v_TextureCoordinate = a_TextureCoordinate;
	
	v_WorldPosition = vec4(u_MMatrix * vec4(a_Position, 1.)).xyz;
	v_WorldNormal = normalize(u_MMatrix * vec4(a_Normal,0.0)).xyz;
	v_WorldTangent = normalize(u_MMatrix * vec4(a_Tangent,0.0)).xyz;

	gl_Position = u_MVPMatrix*vec4(a_Position, 1.0);
}