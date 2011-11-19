#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <Box2D/Box2D.h>
#include <stdio.h>
//#define printf(...) (__VA_ARGS__)

 // CONSTANTS
static const float UNPX = 16.0;
static const float PX  = 1/UNPX;
static const float FPS = 60.0;
static const double pi = M_PI;
static const float gravity = -30;
 // SETTINGS
float window_scale = 2.0;  // 1.0 = 320x240, 2.0 = 640x480
bool window_fullscreen = false;
float cursor_scale = 1.0;  // Higher = faster cursor



 // UTILITY
#include "debug.c++"
#include "vec.h"
#include "math.h"


#define HEADER

#include "coll.c++"
 // RESOURCES
#include "img.c++"
#include "snd.c++"
#include "tiles.c++"
#include "rooms.c++"
#include "stats.c++"
#include "poses.c++"
#include "items.c++"

 // GRAPHICS
#include "drawing.c++"
#include "text.c++"

 // ACTORS
#include "actors.c++"
#include "Actor.c++"
#include "Object.c++"
#include "Walking.c++"
#include "Item.c++"
#include "Rata.c++"
#include "Room.c++"
#include "types.c++"

 // MISC
#include "loop.c++"
#include "map.c++"
#include "query.c++"
#include "bullet.c++"
#include "buttons.c++"
#undef HEADER


#include "game.c++"


 // GRAPHICS
#include "drawing.c++"
#include "text.c++"

 // ACTORS
#include "Actor.c++"
#include "Object.c++"
#include "Walking.c++"
#include "Rata.c++"
#include "Item.c++"
#include "Room.c++"
#include "actors.c++"
#include "misc.c++"
#include "types.c++"
 // MISC
#include "coll.c++"
#include "map.c++"
#include "query.c++"
#include "bullet.c++"
#include "buttons.c++"
#include "items.c++"



#include "loop.c++"




