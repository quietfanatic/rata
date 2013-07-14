
#include "../../core/inc/game.h"
#include "../../hacc/inc/files.h"
#include "../../vis/inc/common.h"

static std::string initial_state = "modules/shell/start.hacc";
static std::string stop_state = "save/last_stop.hacc";

int main () {
    core::load(initial_state);
    core::start(vis::render);
    hacc::File(initial_state).rename(stop_state);
    hacc::save(hacc::File(stop_state));
    fprintf(stderr, "Quit successfully\n");
}




