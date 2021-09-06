#version 330 core

layout(std140) uniform u_fragment
{
    vec2 iResolution;
    float iTime;
} u_frag;

uniform sampler2D iChannel0;
uniform sampler2D iChannel1;

out vec4 fragColor;
in vec2 fragCoord;

void main()
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 aspect = vec2( iResolution.x / iResolution.y, 1.0 );
    vec2 uv = fragCoord/iResolution.xy;
    
    // Define the fold.
    vec2 origin = vec2(0.01, 0.2);// vec2( 0.6 + 0.4 * sin( iTime * 0.2 ), 0.5 + 0.5 * cos( iTime * 0.13 ) ) * aspect;
    vec2 normal = normalize( vec2( 1.0, 2.0 * sin( iTime * 0.3 ) ) * aspect );

    // Sample texture.
    vec3 col = texture( iChannel0, uv ).rgb; // Front color.
    
    // Check on which side the pixel lies.
    vec2 pt = uv * aspect - origin;
    float side = dot( pt, normal );
    if( side > 0.0 ) {
        col *= 0.25; // Background color (peeled off).        
            
        float shadow = smoothstep( 0.0, 0.05, side );
        col = mix( col * 0.6, col, shadow );
    }
    else {
        // Find the mirror pixel.
        pt = ( uv * aspect - 2.0 * side * normal ) / aspect;
        
        // Check if we're still inside the image bounds.
        if( pt.x >= 0.0 && pt.x < 1.0 && pt.y >= 0.0 && pt.y < 1.0 ) {
            vec4 back = texture( iChannel0, pt ); // Back color.
            back.rgb = back.rgb * 0.25 + 0.75;
            
            float shadow = smoothstep( 0.0, 0.2, -side );
            back.rgb = mix( back.rgb * 0.2, back.rgb, shadow );
            
            // Support for transparency.
            col = mix( col, back.rgb, back.a );
        }
    }
    
    // Output to screen
    fragColor = vec4(col,1.0);
}