#version 150

#if __VERSION__ >= 130
   #define varying in
   out vec4 mgl_FragColor;
   #define texture2D texture
 #else
   #define mgl_FragColor gl_FragColor
#endif


varying vec3 v_color;

void main() {
    mgl_FragColor = vec4(v_color, 1.0);
}
