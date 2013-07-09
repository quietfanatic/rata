
#include "../../core/inc/game.h"
#include "../../hacc/inc/files.h"

int main () {
    core::load("modules/rata/test_state_out.hacc");
    core::start();
    hacc::File("modules/rata/test_state_out.hacc").rename("modules/rata/test_state_out.hacc");
    hacc::save(hacc::File("modules/rata/test_state_out.hacc"));
    fprintf(stderr, "Quit successfully\n");
}




