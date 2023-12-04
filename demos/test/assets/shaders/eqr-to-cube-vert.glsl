#version 330 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_uv;

uniform vec4 u_data[4];

out vec3 position;

void main()
{
    mat4 vp = mat4(u_data[0], u_data[1], u_data[2], u_data[3]);
    position = a_position;
    gl_Position = vp * vec4(position, 1.0);
}
