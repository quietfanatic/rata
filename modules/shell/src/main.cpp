
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

void step () {
    ent::run_minds();
    phys::space.run();
    vis::camera_pos = geo::update_camera();
}

int main () {
    using namespace core;
    using namespace hacc;
     // Set up window
    load(File(main_file));
    window->step = step;
    window->render = vis::render;
    window->before_next_frame([](){
        load(File(initial_state));
    });
    core::trap_cursor = true;
     // Run
    phys::space.start();
    window->start();
     // After window closes
    File(initial_state).rename(stop_state);
    save(File(stop_state));
    fprintf(stderr, "Quit successfully\n");
}




