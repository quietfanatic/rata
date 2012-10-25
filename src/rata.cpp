
#include "top.cpp"
#include "../t/testfiles.cpp"
#include "../tmp/testlist.cpp"

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
     // Ghetto option parsing
    if (argc >= 3 && 0==strcmp(argv[1], "--test")) {
        do_test(argv[2]);
        exit(0);
    }

	dbg("Starting\n");
	glfwInit();
	draw::init();
	draw::load_images();
	input::init();
	main_loop();
	return 0;
}

