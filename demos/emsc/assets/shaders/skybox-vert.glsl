#version 100
precision mediump float;

uniform vec4 u_shared_data[4];

attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec2 a_uv;

varying vec3 position;

void main()
{
    mat4 vp = mat4(u_shared_data[0], u_shared_data[1], u_shared_data[2], u_shared_data[3]);
    position = a_position;
    gl_Position = vec4(vp * vec4(position, 1.0)).xyww;
}
