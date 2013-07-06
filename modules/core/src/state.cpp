
#include <stdexcept>
#include "../../hacc/inc/everything.h"
#include "../inc/state.h"
#include "../inc/game.h"
#include "../inc/commands.h"

namespace core {

    bool load_state (std::string filename) {
        try {
            if (!initialized) init();
            hacc::load(filename);
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
            hacc::save(filename);
            return true;
        } catch (hacc::X::Error& e) {
            printf("Failed to save state due to hacc error: %s\n", e.what());
            return false;
        } catch (std::exception& e) {
            printf("Failed to save state due to an exception: %s\n", e.what());
            return false;
        }
    }

} using namespace core;

