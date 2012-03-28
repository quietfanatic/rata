#include <stdio.h>
#include <SOIL/SOIL.h>
#include <GL/glfw.h>
#include <SFML/Audio.hpp>
#include <Box2D/Box2D.h>
//#define printf(...) (__VA_ARGS__)

 // CONSTANTS
const float UNPX = 16.0;
const float PX  = 1/UNPX;
const float FPS = 60.0;
const double pi = M_PI;
const float gravity = -30;
 // SETTINGS
uint window_scale = 3;  // 1 = 320x240, 2 = 640x480, 3 = 960x720
bool window_fullscreen = false;
float cursor_scale = 1.0;  // Higher = faster cursor



int frame_number = -1;

 // UTILITY
#include "debug.c++"
#include "math.h"


#include "header.h"


#define HEADER

 // RESOURCES
#include "img.c++"
#include "snd.c++"
#include "poses.c++"

#undef HEADER


#include "game.c++"


 // GRAPHICS
#include "graphics.c++"
#include "text.c++"

 // ACTORS
#include "Actor.c++"
#include "Spatial.c++"
#include "Object.c++"
#include "Walking.c++"
#include "Item.c++"
#include "items.c++"
#include "Rata.c++"
#include "tiles.c++"
#include "Tilemap.c++"
#include "misc.c++"
#include "enemies.c++"
#include "types.c++"
 // MISC
#include "camera.c++"
#include "rooms.c++"
#include "coll.c++"
#include "map.c++"
#include "query.c++"
#include "bullet.c++"
#include "hud.c++"



#include "loop.c++"




