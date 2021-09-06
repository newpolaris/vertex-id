#version 150

#if __VERSION__ >= 130
   #define attribute in
   #define varying out
#endif

attribute vec3 a_position;
attribute vec2 a_texcoord;

varying vec2 v_texcoord;

uniform mat4 u_MVP;

void main()
{
	vec4 position = u_MVP * vec4(a_position, 1.0);
	gl_Position = position;

	v_texcoord = a_texcoord;
}

