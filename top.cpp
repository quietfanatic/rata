
#include <stdio.h>

#include <SOIL/SOIL.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glfw.h>
#include <SFML/Audio.hpp>
#include <Box2D/Box2D.h>


#define HEADER
#include "debug.cpp"
#include "utility.cpp"
#include "serialize.cpp"
#include "Actor.cpp"
#include "draw.cpp"
#undef HEADER

int frame_number = -1;
int window_scale = 1;

#include "Actor.cpp"
#include "draw.cpp"



