#version 150

#if __VERSION__ >= 130
   #define varying in
   out vec4 mgl_FragColor;
   #define texture2D texture
 #else
   #define mgl_FragColor gl_FragColor
#endif

varying vec3 v_normal;
varying vec2 v_texcoord;

uniform vec3 u_diffuse;

void main() {
    vec3 normal = normalize(v_normal);
    vec3 lightDir = normalize(vec3(0.0, 1.0, -1.0));
    float diffuse = max(dot(normal, lightDir), 0.0);
    mgl_FragColor = vec4(vec3(diffuse), 0.5);
}
