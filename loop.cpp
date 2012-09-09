
#ifdef HEADER

extern int frame_numer;
extern Actor* prime_mover;
void main_loop ();

#else

int frame_number = -1;

void main_loop () {
	double draw_latency = 0;
	for (;;) {
		frame_number++;
		draw_latency -= 1*FR;
		if (draw_latency > 1*FR) {
			dbg_timing("Skipping frame.\n");
		}
		else {
			 // Draw all the things
			draw::start();
			prime_mover.draw();
			draw::finish();
		}
		 // FPS cap.
		draw_latency += glfwGetTime();
		glfwSetTime(0);
		if (draw_latency < 0) glfwSleep(-draw_latency);
		dbg_timing("%f\n", draw_latency);
		glfwSwapBuffers();

		input::check();
		prime_mover.act();
		 // TODO: physics
		prime_mover.react();
	}
}

#endif

