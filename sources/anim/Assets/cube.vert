#version 150

#if __VERSION__ >= 130
   #define attribute in
   #define varying out
#endif

attribute vec3 a_position;
attribute vec3 a_color;

varying vec3 v_color;

uniform mat4 u_transform; 

void main() {
    v_color = a_color;
    gl_Position = u_transform * vec4(a_position, 1.0);
}
