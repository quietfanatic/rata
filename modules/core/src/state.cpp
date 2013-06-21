
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
        for (auto p = things.first(); p; p = p->next()) {
            p->start();
        }
    }
    Game_State::~Game_State () {
        things.destroy_all();
        for (auto p : pop_culture)
            delete p;
    }

    bool load_state (std::string filename) {
        try {
            if (current_state) delete current_state;
            hacc::clear_incantations();  // important to deallocate rooms!
            if (!initialized) init();
            current_state = hacc::new_from_file<Game_State>(filename);
            current_state->start();
            return true;
        } catch (hacc::Error& e) {
            printf("Failed to load state due to hacc error: %s\n", e.what());
            return false;
        } catch (std::exception& e) {
            printf("Failed to load state due to an exception: %s\n", e.what());
            return false;
        }
    }

    bool save_state (std::string filename) {
        try {
            hacc::file_from(filename, *current_state, 4);
            return true;
        } catch (hacc::Error& e) {
            printf("Failed to save state due to hacc error: %s\n", e.what());
            return false;
        } catch (std::exception& e) {
            printf("Failed to save state due to an exception: %s\n", e.what());
            return false;
        }
    }

} using namespace core;

HCB_BEGIN(Game_State)
    attr("things", member(&Game_State::things));
HCB_END(Game_State)

HCB_BEGIN(Stateful)
    pointee_policy(hacc::ALWAYS_FOLLOW);
HCB_END(Stateful)

struct Load_Command : Command {
    std::string filename;
    void operator () () {
        load(filename);
    }
};
HCB_BEGIN(Load_Command)
    base<Command>("load");
    elem(member(&Load_Command::filename));
HCB_END(Load_Command)

struct Save_Command : Command {
    std::string filename;
    void operator () () {
        save(filename);
    }
};
HCB_BEGIN(Save_Command)
    base<Command>("save");
    elem(member(&Save_Command::filename));
HCB_END(Save_Command)

