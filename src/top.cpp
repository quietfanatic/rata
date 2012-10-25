
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#define __STDC_FORMAT_MACROS  // The 'long' and 'short' system is so dumb
#include <inttypes.h>
#include <string>  // I give up.

#include <SOIL/SOIL.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glfw.h>
#include <SFML/Audio.hpp>
#include <Box2D/Box2D.h>
#include "khash.h"

#define HEADER
#include "debug.cpp"
#include "utility.cpp"
#include "serialize.cpp"
//#include "phys.cpp"
#include "draw.cpp"
#include "input.cpp"
#include "loop.cpp"
#undef HEADER

int window_scale = 1;

#include "input.cpp"
#include "serialize.cpp"
#include "loop.cpp"
//#include "phys.cpp"
#include "../tmp/imgs.cpp"
#include "draw.cpp"

