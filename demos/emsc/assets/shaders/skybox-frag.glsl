#version 100
precision mediump float;

#define SKYBOX_CUBEMAP u_common_cube[0]

uniform samplerCube u_common_cube[1];

varying vec3 position;

void main() 
{
    gl_FragColor = textureCube(SKYBOX_CUBEMAP, position);
}
