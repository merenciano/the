#version 330

uniform vec4 u_data[9];

out vec4 FragColor;

void main()
{
	FragColor = u_data[8];
}