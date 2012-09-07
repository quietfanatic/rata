
#include <stdio.h>

#include <SOIL/SOIL.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glfw.h>
#include <SFML/Audio.hpp>
#include <Box2D/Box2D.h>


#define HEADER
#include "debug.c++"
#include "utility.c++"
#include "serialize.c++"
#undef HEADER

uint frame_number;


