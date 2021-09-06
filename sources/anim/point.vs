#version 150

#if __VERSION__ >= 130
   #define attribute in
   #define varying out
#endif

attribute vec4 a_position;
attribute vec3 a_color;
attribute float a_size;

varying vec3 v_color;

void main() {
    gl_PointSize = a_size;
    v_color = a_color;
    gl_Position = a_position;
}
