
#ifdef HEADER

extern int frame_number;
extern bool paused;
void main_loop ();
void toggle_pause ();
void quit_game ();

#else

int frame_number = -1;
bool paused = false;

void main_loop () {
	glfwDisable(GLFW_AUTO_POLL_EVENTS);
	double draw_latency = 0;
	for (;;) {
		frame_number++;
		draw_latency -= 1*FR;
		if (draw_latency > 1*FR) {
			dbg_timing("Skipping frame.\n");
		}
		else { // Draw all the things
			draw::start();
            dbg("Draw phase.\n");
			draw::finish();
		}
		 // FPS cap.
		draw_latency += glfwGetTime();
		glfwSetTime(0);
		if (draw_latency < 0) glfwSleep(-draw_latency);
		dbg_timing("%f\n", draw_latency);
		glfwSwapBuffers();
		draw::check_error();

		input::count();
		if (paused) {
			glfwWaitEvents();
		}
		else {
			glfwPollEvents();
			dbg("Entity phase\n");
			dbg("Physics phase\n");
			dbg("Reaction phase?\n");
		}
	}
}
void toggle_pause () {
	if ((paused = !paused)) {
	}
}
void quit_game () {
	glfwTerminate();
	exit(0);
}

#endif

