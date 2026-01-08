#version 430 core

layout (location = 0) in vec2 aCoord;
layout (location = 1) in vec2 aTexCoord;

layout(std430, binding=0) readonly buffer positionsSSBO { vec2 positions[]; };

layout (location = 0) uniform float uScreenWidth;
layout (location = 1) uniform float uScreenHeight;
layout (location = 2) uniform float uRadius;  // Particle radius in pixels

layout (location = 0) out vec2 texCoord;
layout (location = 1) out float colorCyclePhase;

const int instancesPerColorCycle = 2000;

void main()
{
    // Scale unit quad (±0.5) to pixel size
    // diameter = 2 * radius pixels
    // In NDC: 1 pixel = 2.0 / screenDimension
    float diameter = 2.0 * uRadius;
    vec2 pixelScale = vec2(2.0 / uScreenWidth, 2.0 / uScreenHeight);
    vec2 scaledCoord = aCoord * diameter * pixelScale;

    // Convert particle position to NDC
    vec2 ndcPos = (positions[gl_InstanceID] * pixelScale) - 1.0;
    ndcPos.y *= -1.0;

    texCoord = aTexCoord;
    colorCyclePhase = float(gl_InstanceID % instancesPerColorCycle ) / instancesPerColorCycle ;
    gl_Position = vec4((scaledCoord + ndcPos), 0.0, 1.0);
}
