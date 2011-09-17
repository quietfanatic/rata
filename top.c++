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


 // Cursor state
struct Cursor {
	float x;  // Position in game coordinates relative to player
	float y;
	img::Image* img;  // Draw this
} cursor;

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

 // Global frame timer
uint frame_number = 0;


 // Game-specific global stuff
Rata* rata = NULL;
char* message = NULL;
char* message_pos = NULL;
char* message_pos_next = NULL;



#include "util.h"
#include "text.c++"
#include "objects.c++"
#include "tiles.c++"
#include "rooms.c++"
#include "main.c++"




int main () {
	load_img();
	main_init();

	/*
	obj::Desc(obj::tilemap, &tiles::testroom).manifest();
	obj::Desc(obj::solid, (b2FixtureDef[]){make_fixdef(make_rect(20, 1), cf::solid, 0.4)}, 10, 0.5).manifest();
	obj::Desc(obj::solid, (b2FixtureDef[]){make_fixdef(make_rect(1, 15), cf::solid, 0.4)}, 0.5, 7.5).manifest();
	obj::Desc(obj::solid, (b2FixtureDef[]){make_fixdef(make_rect(1, 15), cf::solid, 0.4)}, 19.5, 7.5).manifest();
	obj::Desc(obj::solid, (b2FixtureDef[]){make_fixdef(make_rect(20, 1), cf::solid, 0.4)}, 10, 14.5).manifest();
	obj::Desc(obj::solid, (b2FixtureDef[]){make_fixdef(make_rect(4, 1), cf::solid, 0.4)}, 3, 6.5).manifest();
	obj::Desc(obj::solid, (b2FixtureDef[]){make_fixdef(make_rect(5, 1), cf::solid, 0.4)}, 8.5, 9.5).manifest();
	obj::Desc(obj::solid, (b2FixtureDef[]){make_fixdef(make_rect(4, 1), cf::solid, 0.4)}, 8, 3.5).manifest();
	obj::Desc(obj::solid, (b2FixtureDef[]){make_fixdef(make_rect(1, 4), cf::solid, 0.4)}, 10.5, 12).manifest();
	obj::Desc(obj::solid, (b2FixtureDef[]){make_fixdef(make_rect(1, 5), cf::solid, 0.4)}, 13.5, 3.5).manifest();
	obj::Desc(obj::solid, (b2FixtureDef[]){make_fixdef(make_rect(2, 1), cf::solid, 0.4)}, 18, 4.5).manifest();
	obj::Desc(obj::rata, NULL, 1.5, 3).manifest();
	obj::Desc(obj::rat, NULL, 16, 10).manifest();
	obj::Desc(obj::rat, NULL, 12, 10).manifest();
	obj::Desc(obj::rat, NULL, 7.5, 12).manifest();
	obj::Desc(obj::crate, NULL, 8.5, 10.5).manifest();
	*/
	room::list[room::test2].start();
	main_loop();
	return 0;
}









