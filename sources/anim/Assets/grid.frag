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
	gl_FragColor = vec4(vec3(0.2), 0.5) * (1.0 - x);
}