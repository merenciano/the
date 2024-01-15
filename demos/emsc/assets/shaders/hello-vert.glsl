#version 100
precision mediump float;

uniform vec4 u_data[9];

attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec2 a_uv;

void main()
{
    mat4 model = mat4(u_data[0], u_data[1],
                        u_data[2], u_data[3]);
    mat4 vp = mat4(u_data[4], u_data[5],
                    u_data[6], u_data[7]);

    gl_Position = vp * model * vec4(a_position, 1.0);
}
