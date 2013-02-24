#version 110

uniform vec2 camera_pos;
uniform vec2 model_pos;

attribute vec2 vert_pos;

void main () {
    gl_Position.xy = (model_pos - camera_pos + vert_pos) / vec2(10.0, 7.5);
    gl_Position.zw = vec2(0.0, 1.0);
}
