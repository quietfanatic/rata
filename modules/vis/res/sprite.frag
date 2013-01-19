#version 110

uniform sampler2D tex;

void main () {
    gl_FragColor = texture2D(tex, gl_TexCoord[0].st);
    if (gl_FragColor.a < 0.5)
        discard;
}
