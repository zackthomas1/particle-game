#version 330 core

#ifdef GL_ES
precision mediump float;
#endif

#define PI 3.14159265359

// Texture coordinate from the vertex shader.
// Typically normalized from [0.0, 1.0]
in vec2 texCoord;
in float colorCyclePhase ; 

out vec4 FragColor;

const vec3 gradientStartColor  = vec3(0.53, 0.96, 0.03);
const vec3 gradientEndColor = vec3(0.96, 0.03, 0.6);

vec3 rgb2hsb( in vec3 c ){
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz),
                 vec4(c.gb, K.xy),
                 step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r),
                 vec4(c.r, p.yzx),
                 step(p.x, c.r));
    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)),
                d / (q.x + e),
                q.x);
}

//  Function from Iñigo Quiles
//  https://www.shadertoy.com/view/MsS3Wc
vec3 hsb2rgb( in vec3 c ){
    vec3 rgb = clamp(abs(mod(c.x*6.0+vec3(0.0,4.0,2.0),
                             6.0)-3.0)-1.0,
                     0.0,
                     1.0 );
    rgb = rgb*rgb*(3.0-2.0*rgb);
    return c.z * mix(vec3(1.0), rgb, c.y);
}


float circle(in vec2 coord, in float radius)
{
    // calculate the distance frrom the center of the quad (0.5,0.5) to the current pixle.
    vec2 dist = coord - vec2(0.5);
    const float transition = 0.1;
    return 1.0 - smoothstep(radius - (radius * transition), 
        radius + (radius * transition), 
        dot(dist, dist) * 4.0);
}

void main()
{
    vec3 pct = vec3(colorCyclePhase);
    // pct.r = smoothstep(0.0,1.0, colorCyclePhase);
    // pct.g = sin(colorCyclePhase*PI);
    // pct.b = pow(colorCyclePhase,0.5);

    vec3 circleColor = mix(rgb2hsb(gradientStartColor), rgb2hsb(gradientEndColor), pct);
    circleColor = hsb2rgb(circleColor);

    float circleAlpha = circle(texCoord, 0.9);
    FragColor = vec4(circleColor,circleAlpha);
}