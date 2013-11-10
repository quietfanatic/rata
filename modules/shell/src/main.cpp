#include <unistd.h>
#include "../../core/inc/window.h"
#include "../../core/inc/input.h"
#include "../../hacc/inc/files.h"
#include "../../phys/inc/phys.h"
#include "../../geo/inc/camera.h"
#include "../../ent/inc/control.h"
#include "../../vis/inc/common.h"

static std::string initial_state = "shell/start.hacc";
static std::string stop_state = "../save/last_stop.hacc";
static std::string main_file = "shell/main.hacc";

 // Global game settings.  Eventually, all modules with configurable settings
 //  are expected to reserve a member of this struct.
struct Settings {
    core::Window window;
    vis::Settings vis;
};
static Settings* settings = NULL;

void step () {
    ent::run_minds();
    phys::space.run();
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
    core::trap_cursor = true;
     // Run
    phys::space.start();
    settings->window.start();
     // After window closes
    File(initial_state).rename(stop_state);
    save(File(stop_state));
    fprintf(stderr, "Quit successfully\n");
}

HCB_BEGIN(Settings)
    name("Settings");
    attr("window", member(&Settings::window).optional());
    attr("vis", member(&Settings::vis).optional());
HCB_END(Settings)


