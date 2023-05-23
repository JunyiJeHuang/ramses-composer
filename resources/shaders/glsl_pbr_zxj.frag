#version 300 es
precision highp float;
#define PI 3.1415926

uniform mat4 u_MMatrix;

uniform vec3 u_Color;
uniform sampler2D u_MainTex;
uniform float u_Metal;
uniform sampler2D u_MetalTex;
uniform float u_Roughness;
uniform sampler2D u_RoughnessTex;
uniform float u_NormalPower;
uniform sampler2D u_NormalTex;
uniform sampler2D u_AOTex;
uniform float u_AOPower;
uniform float u_Opacity;
uniform samplerCube u_Cube;
uniform float u_CubeMapRotation;

uniform vec3 u_LightDir;
//const vec3 u_LightDir = vec3(0.1,1.,0.5);
uniform vec3 u_LightCol;
//const vec3 u_LightCol = vec3(1.);
uniform vec3 u_Ambient;
//const vec3 u_Ambient = vec3(1.);

in vec2 v_TextureCoordinate;
in vec3 v_WorldPosition;
in vec3 v_WorldNormal;
in vec3 v_WorldTangent;

out vec4 FragColor;

struct OutPut
{
	vec4 BaseCol;
	vec4 Normal;
    float Smooth;
	float Metal;
	float AO;
};

OutPut StandardLight()
{
    OutPut o;
	
	vec2 uv = v_TextureCoordinate;

	vec4 col = texture(u_MainTex, uv);
	
	o.BaseCol = col * vec4(u_Color, 1.);
	o.Smooth = max(0., 1.-texture(u_RoughnessTex, uv).r*u_Roughness);
	o.Metal = u_Metal*texture(u_MetalTex, uv).r;
	o.Normal = texture(u_NormalTex, uv);
	o.AO = pow(texture(u_AOTex, uv).r,u_AOPower);
	o.BaseCol.a *= u_Opacity;

	return o;
}

mat2 mm2(float a)
{
	float c = cos(a), s = sin(a);
	return mat2(c,-s,s,c);
}

float lambert(vec3 normal, vec3 lightDir, vec3 viewDir,float Smooth)
{
	//float lam = max(dot(normal, lightDir), 0.);  //old function

	float k = pow((1.-Smooth)+1., 2.)/8.;  //Micro surface
    float lam = (dot(normal, lightDir)/mix(dot(normal, lightDir), 1., k)) * (dot(normal, viewDir)/mix(dot(normal, viewDir), 1., k));
    lam *= smoothstep(0., 0.01, dot(normal, lightDir));

	return clamp(lam, 0., 1.);
}

float blinnPhone(vec3 normal, vec3 lightDir, vec3 viewDir, float roughness)
{
	vec3 halfDir = normalize(viewDir + lightDir);   
	float bli = roughness*roughness / (PI * pow(pow(dot(normal, halfDir), 2.)*(roughness*roughness-1.)+1., 2.));

	return bli;
}

float fresnel(vec3 normal, vec3 viewDir, vec3 surfaceCol, float metal)
{
	//float fre = max(pow(1.- clamp(dot(normal, viewDir),0.,1.),  power), 0.);  //old function
	vec3 F0 = mix(vec3(0.04), surfaceCol, metal);
	float fre = clamp(mix(pow( 1.-dot(normal, viewDir), 1.), 1., length(F0)),0.,1.);
    return fre;
}

void main()
{
	OutPut i;
	i = StandardLight();

	vec3 lightDir = normalize(u_LightDir);

    vec3 viewDir = normalize(vec3(0., 0., 10.) - v_WorldPosition);

	vec3 N = normalize(v_WorldNormal);
	vec3 T = normalize(v_WorldTangent - dot(v_WorldNormal, v_WorldTangent) * v_WorldNormal);
	vec3 B = cross(N, T);
	
	mat3 TBN = mat3(T,B,N);

	vec3 normal = normalize(i.Normal.xyz * 2. - 1.);
	normal.y *= u_NormalPower;
	normal = normalize(TBN * normal);

	float Lambert = lambert(normal, lightDir, viewDir, i.Smooth);

	float Fresnel = fresnel(normal, viewDir, i.BaseCol.rgb, i.Metal); 

    float BlinnPhone = blinnPhone(normal, lightDir, viewDir, clamp(1.-i.Smooth,0.001,1.));

	vec3 ReflDir = reflect(-viewDir, normal);
	ReflDir.xz *= mm2(u_CubeMapRotation);
	vec3 ReflCol = textureLod(u_Cube, ReflDir, 15.-min(pow(i.Smooth, 8.), 15.) * 15.).rgb;
	vec3 AmbientCol = u_Ambient.rgb * ReflCol;

	vec3 L1 = u_LightCol;

	vec3 DiffusedCol = i.BaseCol.rgb * (Lambert * L1 + u_Ambient.rgb * textureLod(u_Cube, ReflDir, 1000.).rgb);
	vec3 RegularCol = mix(mix(vec3(0.), AmbientCol, Fresnel), AmbientCol*i.BaseCol.rgb, i.Metal) + mix(L1*BlinnPhone, L1*BlinnPhone*i.BaseCol.rgb, i.Metal);

    vec3 finCol = mix(DiffusedCol, DiffusedCol*0.5*(1.-i.Smooth), i.Metal) + mix(RegularCol*0.5,  RegularCol, i.Metal)*pow(i.Smooth,0.5);

	FragColor.rgb = mix(vec3(0.), finCol, clamp(i.AO, 0., 1.));

	FragColor.a = clamp(i.BaseCol.a + length(RegularCol), 0., 1.);
}