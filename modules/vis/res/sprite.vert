#version 110

uniform vec2 camera_pos;
uniform vec3 model_pos;
uniform vec2 model_scale;

attribute vec2 vert_pos;
attribute vec2 vert_tex;

varying vec2 tex_coords;

void main () {
    gl_Position.xy = (model_pos.xy - camera_pos + vert_pos * model_scale) / vec2(10.0, 7.5);
    gl_Position.zw = vec2(model_pos.z, 1.0);
    tex_coords = vert_tex;
}
