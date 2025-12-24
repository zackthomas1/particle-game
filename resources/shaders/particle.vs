#version 430 core
layout (location = 0) in vec2 aCoord;

layout (location = 1) in vec2 aPosition;
// layout (location = 2) in vec2 aSize; 
// layout (location = 3) in vec4 aColor; 

out vec4 fColor;

void main()
{
    fColor = vec4(1.0,0.0,0.0,1.0);
    gl_Position = vec4((aCoord + aPosition), 0.0, 1.0);
}
