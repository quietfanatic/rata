
#include "top.cpp"

Actor* prime_mover = new Tester;

int main (int argc, char** argv) {
     // Change directory to program
    char* lastslash = NULL;
    for (char* p = argv[0]; *p; p++) {
        if (*p == '/') lastslash = p;
    }
    if (lastslash) {
        *lastslash = 0;  // haha, modifying argv[0], probably bad
        if (chdir(argv[0]) == -1) {
            printf("chdir() failed with %d (%s)\n", errno, strerror(errno));
        }
    }  // else you'd better know what you're doing cause I don't

	dbg("Starting\n");
	glfwInit();
	draw::init();
	draw::load_images();
	input::init();
	main_loop();
	return 0;
}

