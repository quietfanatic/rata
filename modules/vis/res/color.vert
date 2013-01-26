#version 110

void main () {
    gl_Position = ftransform();
    gl_FrontColor = gl_Color;
}
