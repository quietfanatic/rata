#version 110

uniform vec2 camera_pos;
uniform vec2 model_pos;
attribute vec2 vert_pos;
attribute vec2 vert_tex;
varying vec2 tex_coords;

const float PX = 1.0/16.0;

void main () {
    gl_Position = (vert_pos * PX + model_pos - camera_pos) / Vec(10.0, 7.5);
    tex_coords = vert_tex / 16.0;
}
