
#include "../../core/inc/state.h"
#include "../../core/inc/game.h"
#include "../../hacc/inc/everything.h"
#include "../../vis/inc/text.h"
#include "../../ent/inc/humans.h"
#include "../../geo/inc/rooms.h"

int main () {
    core::load("modules/rata/test_state.hacc");
    ent::Biped* rata = hacc::File("modules/rata/test_state.hacc").data().attr("rata");
    geo::geography().behold(rata);
    core::start();
}




