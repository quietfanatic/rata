
#include "../../core/inc/state.h"
#include "../../core/inc/game.h"
#include "../../hacc/inc/everything.h"
#include "../../vis/inc/text.h"

struct LCF : core::Layer {
    LCF () : core::Layer ("X.M") { }
    void init () {
        vis::console_font = hacc::reference_file<vis::Font>("modules/rata/res/monospace.font");
    }
} lcf;

int main () {
    core::load("modules/rata/test_state.hacc");
}




