
#include <stdexcept>
#include "../../hacc/inc/everything.h"
#include "../inc/state.h"
#include "../inc/game.h"
#include "../inc/commands.h"

using namespace core;

HCB_BEGIN(Game_State)
    attr("things", member(&Game_State::things));
HCB_END(Game_State)

HCB_BEGIN(Stateful)
    pointee_policy(hacc::ALWAYS_FOLLOW);
HCB_END(Stateful)

namespace core {

    Game_State* current_state = NULL;

    void Game_State::start () {
        if (started) {
            throw std::logic_error("Error: something tried to start the game state a second time.");
        }
        started = true;
        for (auto p = things.first(); p; p = p->next()) {
            p->start();
        }
    }

    void load (std::string filename) {
        init();
        stop();
        try {
            if (current_state) delete current_state;
            current_state = hacc::new_from_file<Game_State>(filename);
        } catch (hacc::Error& e) {
            printf("Failed to load state due to hacc error: %s\n", e.what());
        } catch (std::exception& e) {
            printf("Failed to load state due to an exception: %s\n", e.what());
        }
        start();
    }

    void save (std::string filename) {
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
            load(filename);
        }
    };

    struct Save_Command : Command {
        std::string filename;
        void operator() () {
            save(filename);
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

