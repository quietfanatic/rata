



 // PHYSICS
b2World* world;
bool paused = false;

 // GRAPHICS
sf::Window* window;


 // INPUT
unsigned char key[400];  // Counts number of frames up to 255
unsigned char button[10];
Vec cursor;  // Relative to player
Vec cursor2;  // Relative to screen
img::Def* cursor_img = NULL;
bool click_taken = false;
bool draw_cursor = false;
bool trap_cursor = false;

 // View
Vec oldfocus;
Vec focus;
Vec camera;
const Vec screen = Vec(20, 15);
float old_camera_rel = 0;
bool camera_jump = false;
bool camera_snap = false;


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

void (* draw_hud )() = NULL;
uint n_buttons = 0;
Button* buttons = NULL;
Button* dragging = NULL;
Vec drag_start;

 // TIMING
//uint frame_number = 0;
sf::Clock frameclock;
float draw_latency = 0;




 // FOR DEBUGGING
bool debug_mode = false;
const uint debug_path_size = 120;
Vec debug_path [debug_path_size];
Color debug_path_color [debug_path_size];
uint debug_path_pos;
Vec oldratapos;
const uint max_debug_points = 32;
uint n_debug_points = 0;
Vec debug_points [max_debug_points];
void reg_debug_point(Vec p) {
	if (n_debug_points == max_debug_points) {
		printf("Warning: Out of debug points.\n");
		return;
	}
	debug_points[n_debug_points++] = p;
}
Line debug_line = Line(Vec::undef, Vec::undef);








