#version 330 core

layout (location = 0) in vec2 aCoord;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec2 aPosition;
// layout (location = 2) in vec2 aSize; 
// layout (location = 3) in vec4 aColor; 

uniform float uScreenWidth;
uniform float uScreenHeight;
uniform float uRadius;  // Particle radius in pixels

out vec2 texCoord;

void main()
{
    // Scale unit quad (±0.5) to pixel size
    // diameter = 2 * radius pixels
    // In NDC: 1 pixel = 2.0 / screenDimension
    float diameter = 2.0 * uRadius;
    vec2 pixelScale = vec2(2.0 / uScreenWidth, 2.0 / uScreenHeight);
    vec2 scaledCoord = aCoord * diameter * pixelScale;

    // Convert particle position to NDC
    vec2 ndcPos = (aPosition * pixelScale) - 1.0;
    ndcPos.y *= -1.0;

    texCoord = aTexCoord;
    gl_Position = vec4((scaledCoord + ndcPos), 0.0, 1.0);
}
