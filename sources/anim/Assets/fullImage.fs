#version 150

#if __VERSION__ >= 130
   #define varying in
   out vec4 mgl_FragColor;
   #define texture2D texture
 #else
   #define mgl_FragColor gl_FragColor
#endif

varying vec2 v_texcoord;

uniform sampler2D u_texture;

void main() {
    mgl_FragColor = texture2D(u_texture, v_texcoord);
}
