#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <Box2D/Box2D.h>
#include <stdio.h>
//#define printf(...) (__VA_ARGS__)

static const float UNPX = 16.0;
static const float PX  = 1/UNPX;
static const float FPS = 60.0;
static const double pi = M_PI;

#include "debug.c++"
#include "vec.h"
#include "math.h"


float gravity = -30;

#include "img.c++"
#include "snd.c++"

#define HEADER
#include "drawing.c++"
#include "stats.c++"
#include "objects.c++"
#include "coll.c++"
#include "bullet.c++"
#include "rooms.c++"
#include "room.c++"
#include "tiles.c++"
#include "loop.c++"
#include "text.c++"
#include "buttons.c++"
#include "query.c++"
#undef HEADER
#include "text.c++"


 // SETTINGS

float window_scale = 2.0;  // 1.0 = 320x240, 2.0 = 640x480
bool window_fullscreen = false;
float cursor_scale = 1.0;  // Higher = faster cursor
bool debug_mode = false;  // Toggle to see object positions and such


 // Physical world for box2d
b2World* world;
bool paused = false;

 // SFML main window
//sf::Image* screen;
sf::RenderWindow* window;
sf::View window_view = sf::View(sf::FloatRect(0, 30, 40, 0));
float viewleft () { return window_view.GetRect().Left; }
float viewright () { return window_view.GetRect().Left + 20; }
 // For some reason it gets top and bottom confused.
float viewtop () { return window_view.GetRect().Top + 15; }
float viewbottom () { return window_view.GetRect().Top; }
float viewwidth () { return 20; }
float viewheight () { return 15; }

 // Debug draw Rata path.
static const uint debug_path_size = 120;
Vec debug_path [debug_path_size];
Color debug_path_color [debug_path_size];
uint debug_path_pos;
Vec oldratapos;

 // Cursor state
struct Cursor {
	float x;
	float y;
	int16 img;  // Draw this
} cursor,  // Relative to player
  cursor2;  // Relative to world
bool click_taken = false;
bool draw_cursor = false;
bool trap_cursor = false;

 // Camera state
struct Camera {
	float x;
	float y;
} camera;
bool camera_jump = false;

 // Global object lists
Actor* active_actors = NULL;  // Actors are linked list
Actor* activation_queue = NULL;
static const uint MAX_BULLETS = 10;
RBullet bullets[MAX_BULLETS];

 // Clickable buttons (for menu, pause screen, etc.)
uint n_buttons = 0;
Button* buttons = NULL;


 // Input state
unsigned char key[400];  // Counts number of frames up to 255
unsigned char button[10];

 // Time
uint frame_number = 0;
sf::Clock frameclock;
float draw_latency = 0;

 // Game-specific global stuff
Rata* rata = NULL;
char* message = NULL;
char* message_pos = NULL;
char* message_pos_next = NULL;


#include "buttons.c++"
#include "drawing.c++"
#include "poses.c++"
#define HEADER
#include "items.c++"
#undef HEADER
#include "objects.c++"
#include "query.c++"
#include "global.c++"
#include "coll.c++"
#include "bullet.c++"
#include "items.c++"
#include "tiles.c++"
#include "loop.c++"








