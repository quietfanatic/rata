#version 110

uniform sampler2D tex;
varying vec2 tex_coords;

void main () {
    gl_FragColor = texture2D(tex, tex_coords);
    gl_FragDepth = 1.0 - gl_FragColor.a;
}
