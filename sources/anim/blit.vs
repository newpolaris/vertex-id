#version 150

#if __VERSION__ >= 130
   #define attribute in
   #define varying out
#endif

attribute vec3 a_position;
attribute vec2 a_texcoord;
varying vec2 v_texcoord;

uniform mat4 u_transform;

void main() {
    v_texcoord = a_texcoord;
    gl_Position = u_transform * vec4(a_position, 1.0);
}
