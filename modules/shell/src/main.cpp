#include <unistd.h>
#include "../../core/inc/window.h"
#include "../../core/inc/commands.h"
#include "../../hacc/inc/files.h"
#include "../../hacc/inc/haccable_standard.h"
#include "../../phys/inc/phys.h"
#include "../../geo/inc/camera.h"
#include "../../ent/inc/control.h"
#include "../../vis/inc/common.h"
#include "../../vis/inc/text.h"
#include "../../vis/inc/light.h"
#include "../../util/inc/integration.h"
#include "../inc/main.h"

using namespace hacc;
using namespace core;

File main_file = File("shell/main.hacc");
File current_state = File("shell/initial-state.hacc");

static std::string state_arg;

static std::string default_state_filename = "shell/initial-state.hacc";
static std::string stop_state_filename = "../save/last_stop.hacc";

struct Hotkeys : core::Listener {
    std::vector<std::pair<int, core::Command>> hotkeys;
    bool Listener_key (int keycode, int action) override {
        if (action == GLFW_PRESS) {
            for (auto& p : hotkeys) {
                if (keycode == p.first) {
                    p.second();
                    return true;
                }
            }
        }
        return false;
    }
    int Listener_trap_cursor () override { return -1; }
    void finish () { activate(); }
};

 // Global game settings.  Eventually, all modules with configurable settings
 //  are expected to reserve a member of this struct.
struct Game {
    bool paused = false;
    core::Command on_start;
    core::Command on_exit;
};
static Game* game = NULL;

void step () {
    if (!game->paused) {
        ent::run_minds();
        phys::space.run();
    }
    geo::camera->Camera_update();
    vis::camera_pos = geo::camera->Camera_pos();
    vis::camera_size = geo::camera->Camera_size();
}

int main (int argc, char** argv) {

    if (argc >= 2)
        state_arg = rel2abs(argv[1]);

    auto here = util::my_dir(argc, argv);
    chdir(here + "/modules");

    game = main_file.data().attr("game");
    window->step = step;
    window->render = vis::render;
    window->before_next_frame([&](){
        game->on_start();
    });
     // Run
    phys::space.start();
    window->start();
     // After window closes
    if (game->on_exit)
        game->on_exit();
    fprintf(stderr, "Quit successfully\n");
}

HACCABLE(Game) {
    name("Game");
    attr("paused", member(&Game::paused).optional());
    attr("on_start", member(&Game::on_start));
    attr("on_exit", member(&Game::on_exit).optional());
}
HACCABLE(Hotkeys) {
    name("Hotkeys");
    delegate(member(&Hotkeys::hotkeys));
    finish([](Hotkeys& v){ v.finish(); });
}

void _pause () {
    game->paused = !game->paused;
}
core::New_Command _pause_cmd ("pause", "Toggle whether game activity occurs.", 0, _pause);

void _lst (std::string s) {
    if (s.empty()) {
        reload(current_state);
    }
    else {
        unload(current_state);
        current_state = File(s);
        load(current_state);
    }
}
core::New_Command _lst_cmd ("lst", "Load a state (throwing away current state), by default reload same state.", 0, _pause);

void _lst_arg (std::string s) {
    if (!state_arg.empty())
        _lst(state_arg);
    else
        _lst(s);
}

core::New_Command _lst_arg_cmd ("lst_arg", "Load the command-line argument state with this as default.", 0, _lst_arg);

void _sst (std::string s) {
    if (!s.empty())
        current_state.rename(s);
    save(current_state);
}

core::New_Command _sst_cmd ("sst", "Save the current state to a file (default same as was loaded from).", 0, _sst);
