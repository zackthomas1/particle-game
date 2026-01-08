#version 430 core
#define LOCALSIZE 16
layout (local_size_x = LOCALSIZE, local_size_y = LOCALSIZE, local_size_z = 1) in;

layout(std430, binding=0) buffer prevPositionsSSBO { vec2 prevPositions[]; };
layout(std430, binding=1) buffer positionsSSBO { vec2 positions[]; };
layout(std430, binding=2) buffer velocitiesSSBO { vec2 velocities[]; };
layout(std430, binding=3) buffer massesSSBO { float masses[]; };

layout(location=0) uniform float uDeltaTime; 
const float timeScale  = 10.0f;
void main()
{
    uint idx = gl_GlobalInvocationID.x;
    uint idy = gl_GlobalInvocationID.y;
    uint i = idx + (idy * gl_NumWorkGroups.x * gl_WorkGroupSize.x);

    vec2 force = vec2(0.0, 9.8);
    float inverseMass = 1.0f / masses[i];

    velocities[i] += force * (inverseMass * uDeltaTime);
    prevPositions[i] = positions[i];
    positions[i] += velocities[i]* uDeltaTime;
}
