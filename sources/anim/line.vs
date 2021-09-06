#version 150

#if __VERSION__ >= 130
   #define attribute in
   #define varying out
#endif

attribute vec4 a_position;
attribute vec3 a_color;
varying vec3 v_color;

void main() {
    gl_Position = vec4(a_position);
    v_color = a_color;
}
