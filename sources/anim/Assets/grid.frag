#version 150

#if __VERSION__ >= 130
   #define varying in
   out vec4 mgl_FragColor;
   #define texture2D texture
 #else
   #define mgl_FragColor gl_FragColor
#endif


uniform float u_Scale;
uniform float u_Res;

varying vec2 v_texcoord;

float grid(vec2 st, float res)
{
	vec2 grid = fract(st);
	return step(res, grid.x) * step(res, grid.y);
}
 
void main()
{
	float scale = u_Scale;
	float resolution = u_Res;

	float x = grid(v_texcoord * scale, resolution);
	mgl_FragColor = vec4(vec3(0.2), 0.5) * (1.0 - x);
}
