
#include "top.cpp"

Actor* prime_mover = new Tester;

int main () {
	dbg("Starting\n");
	glfwInit();
	draw::init();
	draw::load_images();
	input::init();
	main_loop();
	return 0;
}


