
#include "../../hacc/inc/haccable_pointers.h"
#include "../inc/state.h"

using namespace core;

HCB_BEGIN(Game_State)
    attr("things", member(&Game_State::things));
HCB_END(Game_State)

HCB_BEGIN(Stateful)
    pointee_policy(hacc::FOLLOW);
HCB_END(Stateful)

namespace core {
    Game_State* current_state = NULL;
}

