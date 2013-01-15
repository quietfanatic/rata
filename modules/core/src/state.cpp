
#include "../../hacc/inc/everything.h"
#include "../inc/state.h"
#include "../inc/commands.h"

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

    void save_state (std::string filename) {
        try {
            hacc::file_from(filename, *current_state);
        } catch (hacc::Error& e) {
            printf("Failed to save state due to hacc error: %s\n", e.what());
        } catch (std::exception& e) {
            printf("Failed to save state due to an exception: %s\n", e.what());
        }
    }

    struct Load_Command : Command {
        std::string filename;
        void operator() () {
            load_state(filename);
        }
    };

    struct Save_Command : Command {
        std::string filename;
        void operator() () {
            save_state(filename);
        }
    };

}

HCB_BEGIN(core::Load_Command)
    base<Command>("load");
    elem(member(&core::Load_Command::filename));
HCB_END(core::Load_Command)

HCB_BEGIN(core::Save_Command)
    base<Command>("save");
    elem(member(&core::Save_Command::filename));
HCB_END(core::Save_Command)

