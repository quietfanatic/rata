#version 110

uniform sampler2D tex;

void main () {
    gl_FragColor = texture2D(tex, gl_TexCoord[0].st);
    gl_FragDepth = 1.0 - gl_FragColor.a;
}
