#version 430 core
layout (location = 0) in vec3 attributePos;

uniform mat4 modelmatrix;
uniform mat4 viewmatrix;
uniform mat4 projectionmatrix;

void main()
{
    gl_Position = projectionmatrix * viewmatrix * modelmatrix * vec4(attributePos, 1.0f);
}
