
#include <stdexcept>
#include "../../hacc/inc/everything.h"
#include "../inc/state.h"
#include "../inc/game.h"
#include "../inc/commands.h"

namespace core {

    Game_State* current_state = NULL;

    static std::vector<Game_Object*(*)()>& celeb_allocators () {
        static std::vector<Game_Object*(*)()> celeb_allocators;
        return celeb_allocators;
    }
    uint allocate_celebrity (Game_Object*(* a )()) {
        celeb_allocators().push_back(a);
        return celeb_allocators().size() - 1;
    }

    Game_State::Game_State () {
        pop_culture.resize(celeb_allocators().size());
        for (uint i = 0; i < celeb_allocators().size(); i++) {
            pop_culture[i] = celeb_allocators()[i]();
        }
    }

    void Game_State::start () {
        if (started) {
            throw std::logic_error("Error: something tried to start the game state a second time.");
        }
        started = true;
        for (auto p : pop_culture)
            p->start();
    }
    void Things::start () {
        for (auto p = things.first(); p; p = p->next()) {
            p->start();
        }
    }
    Game_State::~Game_State () {
        for (auto p : pop_culture)
            delete p;
    }

    bool load_state (std::string filename) {
        try {
            if (current_state) delete current_state;
            if (!initialized) init();
            current_state = hacc::File(filename).data().attr("state");
            current_state->start();
            return true;
        } catch (hacc::X::Error& e) {
            printf("Failed to load state due to hacc error: %s\n", e.what());
            return false;
        } catch (std::exception& e) {
            printf("Failed to load state due to an exception: %s\n", e.what());
            return false;
        }
    }

    bool save_state (std::string filename) {
        try {
             // TODO: this is wrong; it only works if the filename
             //  has not been changed.
            save(hacc::File(filename));
            return true;
        } catch (hacc::X::Error& e) {
            printf("Failed to save state due to hacc error: %s\n", e.what());
            return false;
        } catch (std::exception& e) {
            printf("Failed to save state due to an exception: %s\n", e.what());
            return false;
        }
    }

    Celebrity<Things> things;

} using namespace core;

HCB_BEGIN(Game_State)
HCB_END(Game_State)

