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

    std::string state = argc >= 2
        ? rel2abs(argv[1])
        : default_state_filename;

    auto here = util::my_dir(argc, argv);
    chdir(here + "/modules");

    game = main_file.data().attr("game");
    vis::default_font = File("shell/res/monospace.hacc").data().attr("font");
    vis::Materials* materials = File("world/res/materials.hacc").data().attr("materials");
    vis::set_materials(materials);

    window->step = step;
    window->render = vis::render;
    window->before_next_frame([&](){
        current_state = File(state);
        load(current_state);
    });
     // Run
    phys::space.start();
    window->start();
     // After window closes
    current_state.rename(stop_state_filename);
    save(current_state);
    fprintf(stderr, "Quit successfully\n");
}

HACCABLE(Game) {
    name("Game");
    attr("paused", member(&Game::paused).optional());
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


