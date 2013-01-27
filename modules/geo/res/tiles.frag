#version 110

uniform sampler2D tex;

void main () {
    gl_FragColor = texture2D(tex, gl_TexCoord[0].st);
    gl_FragDepth = gl_FragColor.a;
}
