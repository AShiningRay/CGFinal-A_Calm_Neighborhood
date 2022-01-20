#version 430 core
out vec4 fragColor;

uniform vec3 lightcolor;

void main()
{
    fragColor = vec4(lightcolor, 1.0f);
}
