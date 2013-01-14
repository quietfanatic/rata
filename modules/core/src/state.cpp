
#include "../../hacc/inc/haccable_pointers.h"
#include "../inc/state.h"

HCB_BEGIN(Game_State)
    attr("things", member(&Game_State::things));
HCB_END(Game_State)

Game_State* current_state = NULL;


