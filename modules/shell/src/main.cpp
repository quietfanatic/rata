
#include "../../core/inc/game.h"
#include "../../hacc/inc/files.h"
#include "../../vis/inc/common.h"

static std::string initial_state = "modules/shell/start.hacc";
static std::string stop_state = "save/last_stop.hacc";
static std::string main_file = "modules/shell/main.hacc";

int main () {
    using namespace core;
    using namespace hacc;
     // Set up window
    load(File(main_file));
    window->render = vis::render;
    window->before_next_frame([](){
        load(File(initial_state));
    });
     // Run
    window->start();
     // After window closes
    File(initial_state).rename(stop_state);
    save(File(stop_state));
    fprintf(stderr, "Quit successfully\n");
}




