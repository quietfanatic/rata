#version 110

uniform sampler2D tex;
uniform vec4 color;
varying vec2 tex_coords;

void main () {
    gl_FragColor = texture2D(tex, tex_coord) * color;
}
