
#include "top.cpp"

Actor* prime_mover = new Tester;

int main () {
	dbg("Starting\n");
	glfwInit();
	input::init();
	draw::init();
	draw::load_images();
	main_loop();
	return 0;
}


