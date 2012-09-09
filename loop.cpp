
#ifdef HEADER

extern int frame_number;
extern bool paused;
extern Actor* prime_mover;
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
			prime_mover->draw();
			draw::finish();
		}
		 // FPS cap.
		draw_latency += glfwGetTime();
		glfwSetTime(0);
		if (draw_latency < 0) glfwSleep(-draw_latency);
		dbg_timing("%f\n", draw_latency);
		glfwSwapBuffers();

		input::check();
		if (paused) {
			glfwWaitEvents();
		}
		else {
			glfwPollEvents();
			prime_mover->act();
			 // TODO: physics
			prime_mover->react();
		}
	}
}
void toggle_pause () {
	if (paused = !paused) {
	}
}
void quit_game () {
	glfwTerminate();
	exit(0);
}

#endif

