#version 150

#if __VERSION__ >= 130
   #define varying in
   out vec4 mgl_FragColor;
   #define texture2D texture
 #else
   #define mgl_FragColor gl_FragColor
#endif

varying vec2 v_texcoord;

// uniform sampler2D u_texture;
uniform vec2 u_viewport;
uniform vec3 u_clearColor;

bool check_coords2(vec2 coord, vec2 block_size) {
    int coord_x = int(coord.x);
    int coord_y = int(coord.y);
    
    if ( ( coord_x % int(block_size.x) ) == 0 )
        return true;
    
    if ( ( coord_y % int(block_size.y) ) == 0 )
 		return true;
    return false;
}

void main() {
    vec4 one_color;
    vec4 two_color;

    vec2 coord = vec2(1, 1) - v_texcoord;
    coord = coord * u_viewport; // vec2(995, 684);
    coord = coord + vec2(5, 5);

  	vec2 block_size = vec2(10.0, 10.0);
    
    one_color.rgb = u_clearColor;
    one_color.a = 1.0;
    
    two_color.r = 0.5;
   	two_color.g = 0.5;
   	two_color.b = 0.5;
    two_color.w = 1.0;

    if (check_coords2(coord,block_size))
        one_color = two_color;
    
	mgl_FragColor  = one_color; 
    // mgl_FragColor = texture2D(u_texture, v_texcoord);
}
