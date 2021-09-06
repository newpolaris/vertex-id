#version 150

#if __VERSION__ >= 130
   #define varying in
   out vec4 mgl_FragColor;
   #define texture2D texture
 #else
   #define mgl_FragColor gl_FragColor
#endif

varying vec2 v_texcoord;
uniform vec2 u_viewport;
uniform vec2 u_center;
uniform vec4 u_color;

void main() {
    float aspect = u_viewport.x / u_viewport.y;
    vec2 as = vec2(1.0, aspect);
    vec2 center = u_center / u_viewport;
    vec2 coord = vec2(v_texcoord.x, 1.0 - v_texcoord.y);
    float r = length((coord - center)/as);
    vec4 color = vec4(0.0, 0.0, 0.0, 0.0);
    if (r < 0.003)
        color = u_color;
    mgl_FragColor = color;
}
