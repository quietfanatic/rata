#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <Box2D/Box2D.h>
#include <stdio.h>
//#define printf(...) (__VA_ARGS__)

 // CONSTANTS
const float UNPX = 16.0;
const float PX  = 1/UNPX;
const float FPS = 60.0;
const double pi = M_PI;
const float gravity = -30;
 // SETTINGS
uint window_scale = 2;  // 1 = 320x240, 2 = 640x480, 3 = 960x720
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
#include "stats.c++"
#include "poses.c++"
#include "items.c++"
#include "rooms.c++"

 // GRAPHICS
#include "drawing.c++"
#include "text.c++"

 // ACTORS
#include "Actor.c++"
#include "Spatial.c++"
#include "Object.c++"
#include "Walking.c++"
#include "misc.c++"
#include "Item.c++"
#include "Rata.c++"
#include "Tilemap.c++"
#include "enemies.c++"
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
#include "Spatial.c++"
#include "Object.c++"
#include "Walking.c++"
#include "Rata.c++"
#include "Item.c++"
#include "Tilemap.c++"
#include "misc.c++"
#include "enemies.c++"
#include "types.c++"
#include "rooms.c++"
 // MISC
#include "coll.c++"
#include "map.c++"
#include "query.c++"
#include "bullet.c++"
#include "buttons.c++"
#include "items.c++"



#include "loop.c++"




