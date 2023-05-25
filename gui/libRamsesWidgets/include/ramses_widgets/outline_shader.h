#ifndef __GRID_FRAG_H__
#define __GRID_FRAG_H__

namespace raco::ramses_widgets {
static const char* vertexShader = R"SHADER(
#version 300 es
precision mediump float;
in vec3 aPosition;
in vec2 aUVSet0;
	
out vec2 vTC0;
uniform mat4 mvpMatrix;
void main() {
	vTC0 = aUVSet0;
	gl_Position = mvpMatrix * vec4(aPosition.xyz, 1.0);
}
)SHADER";


static const char* fragmentShader = R"SHADER(
#version 300 es
precision mediump float;
in vec2 vTC0;
uniform sampler2D uTex0;
out vec4 FragColor;

void main() {
	vec3 clr0 = texture(uTex0, vTC0).rgb;
    FragColor = vec4(clr0,1.0);
}
)SHADER";

static const char* fragmentShaderBlur = R"SHADER(
#version 300 es
precision highp float;
out vec4 fragColor;
uniform sampler2D uTex0;
uniform mediump vec2 kzTextureSize0;
uniform lowp vec2 BlurDirection;
uniform float BlurRadius;
in vec2 vTC0;
vec4 gaussianBlur(mediump vec2 coord, lowp vec2 dir) {
	float GAUSSIAN_KERNEL[9];
	GAUSSIAN_KERNEL[0] = 0.028532;
	GAUSSIAN_KERNEL[1] = 0.067234;
	GAUSSIAN_KERNEL[2] = 0.124009;
	GAUSSIAN_KERNEL[3] = 0.179044;
	GAUSSIAN_KERNEL[4] = 0.20236; 
	GAUSSIAN_KERNEL[5] = 0.179044;
	GAUSSIAN_KERNEL[6] = 0.124009;
	GAUSSIAN_KERNEL[7] = 0.067234;
	GAUSSIAN_KERNEL[8] = 0.028532;
	vec2 texel = 1.0 / kzTextureSize0;
	vec4 sum = vec4(0.0);
	vec2 tc = coord;
	float blur = BlurRadius;
	float hstep = dir.x * texel.x;
	float vstep = dir.y * texel.y;
	for (int i = 0; i < 9; i++) {
	float pixelOffset = (float(i) - floor(9.0 * 0.5));
	mediump vec2 coord = vec2(tc.x + pixelOffset * blur * hstep, tc.y + pixelOffset * blur * vstep);
	sum += texture(uTex0, coord) * GAUSSIAN_KERNEL[i];
}

return sum;
}

void main() {
	fragColor = gaussianBlur(vTC0, BlurDirection);
}
)SHADER";

static const char* fragmentShaderOutline = R"SHADER(
#version 300 es
precision mediump float;
in vec2 vTC0;
uniform sampler2D u_MainTex;
uniform sampler2D u_BlurTex;
uniform vec4 u_ourlineColor;

out vec4 FragColor;

void main() {
	vec4 scene  = texture(u_MainTex,vTC0);
	vec4 blur  = texture(u_BlurTex,vTC0);
	if(scene.r != 0.0 || scene.g != 0.0 || scene.b != 0.0)
	{
		FragColor = vec4(0.0);
	}
	else
    {
        FragColor = blur*1.5;
	}
}
)SHADER";

static const char* colorVertexShader = R"SHADER(
#version 300 es
precision mediump float;
in vec3 a_Position;
uniform mat4 u_MMatrix;
uniform mat4 u_VMatrix;
uniform mat4 u_PMatrix;
void main() {
    gl_Position = u_PMatrix * u_VMatrix *u_MMatrix * vec4(a_Position.xyz, 1.0);
}
)SHADER";

static const char* colorFragmentShader = R"SHADER(
#version 300 es
precision mediump float;
out vec4 FragColor;
void main() {
    FragColor = vec4(1.0, 1.0, 0.0, 1.0);
}
)SHADER";

}
#endif  // __GRID_FRAG_H__
