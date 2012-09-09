namespace input {
#ifdef HEADER

enum {
	LEFT,
	RIGHT,
	JUMP,
	CROUCH,
	ACTION,
	LOOK,
	AIM,
	FIRE,
	PAUSE,
	N_INPUTS
};

struct Inputs {
	uint left;
	uint right;
	uint jump;
	uint crouch;
	uint action;
	uint look;
	uint aim;
	uint fire;
	uint pause;
	const uint n = N_INPUTS;
	uint& operator [] (uint i) { return ((uint*)this)[i]; }
} inputs;
int8 key_map [400][2];
int8 btn_map [10][2];

void init ();
void check ();

#else

int GLFWCALL key_cb (int keycode, int action) {
	if (action == GLFW_PRESS) {
		switch (keycode) {
			case GLFW_KEY_ESC: quit_game();
			default:
		}
	}
	if (keycode < 400) {
		if (key_map[keycode][0] > -1)
			inputs[key_map[keycode][0]] = (action == GLFW_PRESS);
		if (key_map[keycode][1] > -1)
			inputs[key_map[keycode][1]] = (action == GLFW_PRESS);
	}
}

int GLFWCALL btn_cb (int btncode, int action) {
	if (btncode < 10) {
		if (k_map[btncode][0] > -1)
			inputs[btn_map[keycode][0]] = (action == GLFW_PRESS);
		if (k_map[btncode][1] > -1)
			inputs[btn_map[keycode][1]] = (action == GLFW_PRESS);
	}
}

int GLFWCALL close_cb () {
	quit_game();
	return true;
}

void init () {
	for (uint i=0; i < 400; i++) {
		key_map[i][0] = -1;
		key_map[i][1] = -1;
	}
	for (uint i=0; i < 10; i++) {
		btn_map[i][0] = -1;
		btn_map[i][1] = -1;
	}
	key_map['A'][0] = LEFT;
	key_map['D'][0] = RIGHT;
	key_map['W'][0] = JUMP;
	key_map['S'][0] = CROUCH;
	key_map[GLFW_KEY_SPACE][0] = ACTION;
	btn_map[GLFW_MOUSE_BUTTON_LEFT][0] = LOOK;
	btn_map[GLFW_MOUSE_BUTTON_RIGHT][0] = AIM;
	key_map[GLFW_KEY_LSHIFT][0] = AIM;
	btn_map[GLFW_MOUSE_BUTTON_LEFT][1] = FIRE;
	key_map['P'][0] = PAUSE;
	glfwSetKeyCallback(key_cb);
	glfwSetMouseButtonCallback(btn_cb);
	glfwSetWindowCloseCallback(close_cb);
}

void check () {
	for (uint i=0; i < inputs.n; i++)
	if (inputs[i]) inputs[i]++;
}

#endif
}
