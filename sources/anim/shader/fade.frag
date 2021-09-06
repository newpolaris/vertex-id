#version 330 core

layout(std140) uniform u_fragment
{
    float alpha;
    float edge0;
    float edge1;
} u_frag;

uniform sampler2D u_sampler;

in vec2 v_texcoord;
out vec4 color_out;

void main()
{
    vec2 texcoord = vec2(v_texcoord.x, v_texcoord.y);
    float alpha = u_frag.alpha;
#if defined(HAS_FADE_EDGE)
    alpha = alpha * smoothstep(u_frag.edge0, u_frag.edge1, 1.0 - texcoord.y);
#endif
    vec3 color = texture(u_sampler, texcoord).rgb;
    color_out = vec4(color.rgb, alpha);
}
