#version 300 es
precision highp float;

uniform vec3 u_SkyCol;
uniform int u_UseCubeMap;
uniform float u_CubeMapRotation;
uniform samplerCube u_Cube;

in vec3 v_WorldPosition;

out vec4 FragColor;

mat2 mm2(float a)
{
	float c = cos(a), s = sin(a);
	return mat2(c,-s,s,c);
}

void main()
{
	vec3 col = u_SkyCol;

    if(u_UseCubeMap == 1)
	{
	    vec3 viewDir = normalize(v_WorldPosition - vec3(0.));
		viewDir.xz *= mm2(u_CubeMapRotation);
	    col = textureLod(u_Cube, viewDir, 0.).rgb;
	}

	FragColor.rgb = col;

	FragColor.a = 1.;
}
