



 // PHYSICS
b2World* world;
bool paused = false;

 // GRAPHICS
sf::RenderWindow* window;
sf::View window_view = sf::View(sf::FloatRect(0, 15 * window_scale, 20 * window_scale, 0));
float viewleft () { return window_view.GetRect().Left; }
float viewright () { return window_view.GetRect().Left + 20; }
float viewtop () { return window_view.GetRect().Top + 15; }
float viewbottom () { return window_view.GetRect().Top; }
float viewwidth () { return 20; }
float viewheight () { return 15; }


 // INPUT
unsigned char key[400];  // Counts number of frames up to 255
unsigned char button[10];
struct Cursor {
	float x;
	float y;
	img::Def* img;  // Draw this
} cursor,  // Relative to player
  cursor2;  // Relative to world
bool click_taken = false;
bool draw_cursor = false;
bool trap_cursor = false;

struct Camera {
	float x;
	float y;
} camera;
bool camera_jump = false;


 // GAME STATE
Actor* global_actors = NULL;
Actor* active_actors = NULL;
Actor* activation_queue = NULL;
Rata* rata = NULL;
room::Def* current_room = NULL;
char* message = NULL;
char* message_pos = NULL;
char* message_pos_next = NULL;

const uint MAX_BULLETS = 10;
RBullet bullets[MAX_BULLETS];

uint n_buttons = 0;
Button* buttons = NULL;

 // TIMING
uint frame_number = 0;
sf::Clock frameclock;
float draw_latency = 0;




 // FOR DEBUGGING
bool debug_mode = false;
const uint debug_path_size = 120;
Vec debug_path [debug_path_size];
Color debug_path_color [debug_path_size];
uint debug_path_pos;
Vec oldratapos;










