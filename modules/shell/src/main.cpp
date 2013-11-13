#include <unistd.h>
#include "../../core/inc/window.h"
#include "../../core/inc/input.h"
#include "../../core/inc/commands.h"
#include "../../hacc/inc/files.h"
#include "../../hacc/inc/haccable_standard.h"
#include "../../phys/inc/phys.h"
#include "../../geo/inc/camera.h"
#include "../../ent/inc/control.h"
#include "../../vis/inc/common.h"

static std::string initial_state = "shell/start.hacc";
static std::string stop_state = "../save/last_stop.hacc";
static std::string main_file = "shell/main.hacc";

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
};

 // Global game settings.  Eventually, all modules with configurable settings
 //  are expected to reserve a member of this struct.
struct Settings {
    core::Window window;
    vis::Settings vis;
    Hotkeys hotkeys;
    bool paused = false;
};
static Settings* settings = NULL;

void step () {
    if (!settings->paused) {
        ent::run_minds();
        phys::space.run();
    }
    vis::camera_pos = geo::update_camera();
}

int main (int argc, char** argv) {
     // Find program's directory, chdir to modules
    if (argc < 1) {
        fprintf(stderr, "Program called with no argv[0]!?\n");
    }
    std::string me = (const char*)argv[0];
    auto lastslash = me.rfind('/');
    if (lastslash != std::string::npos) {
        auto modules_dir = me.substr(0, lastslash) + "/modules";
        if (chdir(modules_dir.c_str()) == -1) {
            fprintf(stderr, "Could not chdir(\"%s\"): %d (%s)\n", modules_dir.c_str(), errno, strerror(errno));
        }
    }

    using namespace hacc;

    settings = File(main_file).data();
    settings->window.step = step;
    settings->window.render = vis::render;
    settings->window.before_next_frame([](){
        load(File(initial_state));
    });
    settings->hotkeys.activate();
     // Run
    phys::space.start();
    settings->window.start();
     // After window closes
    File(initial_state).rename(stop_state);
    save(File(stop_state));
    fprintf(stderr, "Quit successfully\n");
}

HACCABLE(Settings) {
    name("Settings");
    attr("window", member(&Settings::window).optional());
    attr("vis", member(&Settings::vis).optional());
    attr("hotkeys", member(&Settings::hotkeys).optional());
    attr("paused", member(&Settings::paused).optional());
}
HACCABLE(Hotkeys) {
    name("Hotkeys");
    delegate(member(&Hotkeys::hotkeys));
}

void _pause () {
    settings->paused = !settings->paused;
}
core::New_Command _pause_cmd ("pause", "Toggle whether game activity occurs.", 0, _pause);


