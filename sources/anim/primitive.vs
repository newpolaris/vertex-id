attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec2 a_texcoord;

varying vec3 v_normal;
varying vec2 v_texcoord;

uniform mat4 u_transform; 

void main() {
    v_normal = vec3(u_transform * vec4(a_normal, 0.0));
    v_texcoord = a_texcoord;
    gl_Position = u_transform * vec4(a_position, 1.0);
}
