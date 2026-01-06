#version 330 core

// Texture coordinate from the vertex shader.
// Typically normalized from [0.0, 1.0]
in vec2 texCoord;

uniform float uScale; 

out vec4 FragColor;

float circle(vec2 coord, float radius)
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
    float circleAlpha = circle(texCoord, 0.9);
    vec3 circleColor = vec3(1.0, 0.0, 0.0);
    FragColor = vec4(circleColor,circleAlpha);
}
