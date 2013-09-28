
#include "../../core/inc/window.h"
#include "../../core/inc/input.h"
#include "../../hacc/inc/files.h"
#include "../../phys/inc/phys.h"
#include "../../geo/inc/camera.h"
#include "../../ent/inc/control.h"
#include "../../vis/inc/common.h"

static std::string initial_state = "modules/shell/start.hacc";
static std::string stop_state = "save/last_stop.hacc";
static std::string main_file = "modules/shell/main.hacc";

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

int main () {
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


