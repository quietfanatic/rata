
#include "../../core/inc/game.h"
#include "../../hacc/inc/files.h"

static std::string initial_state = "modules/rata/test_state.hacc";
static std::string stop_state = "save/last_stop.hacc";

int main () {
    core::load(initial_state);
    core::start();
    hacc::File(initial_state).rename(stop_state);
    hacc::save(hacc::File(stop_state));
    fprintf(stderr, "Quit successfully\n");
}




