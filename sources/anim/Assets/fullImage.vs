#version 150

#if __VERSION__ >= 130
   #define attribute in
   #define varying out
#endif

attribute vec2 a_position;
attribute vec2 a_texcoord;
varying vec2 v_texcoord;

uniform vec2 u_scale;

void main() {
    v_texcoord = a_texcoord;
    gl_Position = vec4(a_position*u_scale, 0.0, 1.0);
}