
#include "../../hacc/inc/everything.h"
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

    void Game_State::exist () {
        for (auto p = things.first(); p; p = p->next()) {
            p->exist();
        }
    }

    void load_state (std::string filename) {
        try {
            Game_State* state = hacc::new_from_file<Game_State>(filename);
            if (state) {
                delete current_state;
                current_state = state;
                current_state->exist();
            }
        } catch (hacc::Error& e) {
            printf("Failed to load state due to hacc error: %s\n", e.what());
        } catch (std::exception& e) {
            printf("Failed to load state due to an exception: %s\n", e.what());
        }
    }

}

