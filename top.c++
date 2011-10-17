#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <Box2D/Box2D.h>
#include <stdio.h>
//#define printf(...) (__VA_ARGS__)

#include "debug.c++"


#define PX (1/16.0)
#define FPS 60.0

#include "img.c++"

#define DEF_ONLY
#include "objects.c++"
#include "rooms.c++"
#include "tiles.c++"
#undef DEF_ONLY


 // SETTINGS

float window_scale = 2.0;  // 1.0 = 320x240, 2.0 = 640x480
bool window_fullscreen = false;
float cursor_scale = 1.0;  // Higher = faster cursor
bool debug_mode = false;  // Toggle to see object positions and such

 // Physical world for box2d
b2World* world;

 // SFML main window
sf::Image* screen;
sf::RenderWindow* window;
sf::View window_view = sf::View(sf::FloatRect(0, 15, 20, 0));
float viewleft () { return window_view.GetRect().Left; }
float viewright () { return window_view.GetRect().Right; }
 // For some reason it gets top and bottom confused.
float viewtop () { return window_view.GetRect().Bottom; }
float viewbottom () { return window_view.GetRect().Top; }
float viewwidth () { return viewright() - viewleft(); }
float viewheight () { return viewtop() - viewbottom(); }

 // Shading
bool screen_shade = false;
sf::Color screen_shade_color = sf::Color(255, 196, 63);
sf::Blend::Mode screen_shade_blend = sf::Blend::Multiply;

 // Cursor state
struct Cursor {
	float x;
	float y;
	img::Image* img;  // Draw this
} cursor,  // Relative to player
  cursor2;  // Relative to world
bool click_taken = false;

 // Camera state
struct Camera {
	float x;
	float y;
} camera;

 // Global object lists
Object* objects_by_depth = NULL;  // Objects are linked list
Object* objects_by_order = NULL;
Object* creation_queue = NULL;

 // Input state
unsigned char key[400];  // Counts number of frames up to 255
unsigned char button[10];

 // Time
uint frame_number = 0;
sf::Clock frameclock;
float lastframe = 0;

 // Game-specific global stuff
Rata* rata = NULL;
char* message = NULL;
char* message_pos = NULL;
char* message_pos_next = NULL;


#ifdef MAPEDITOR
 // Editor stuff
bool mapeditor = false;
int16 selected_tile = 0;
bool flip_tile = false;
float tilepicker_width = 1.5;
#endif

#include "util.h"
#include "text.c++"
#include "poses.c++"
#define DEF_ONLY
#include "items.c++"
#undef DEF_ONLY
#include "objects.c++"
#include "items.c++"
#include "tiles.c++"
#include "rooms.c++"
#include "loop.c++"








