#version 330 core

uniform vec4 u_data[9];

layout(location=0) in vec3 a_position;
layout(location=1) in vec3 a_normal;
layout(location=2) in vec2 a_uv;

void main()
{
    mat4 model = mat4(u_data[0], u_data[1],
                        u_data[2], u_data[3]);
    mat4 vp = mat4(u_data[4], u_data[5],
                    u_data[6], u_data[7]);

    gl_Position = vp * model * vec4(a_position, 1.0);
}
