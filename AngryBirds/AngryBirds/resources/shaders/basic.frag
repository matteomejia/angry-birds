#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

void main()
{
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
} 