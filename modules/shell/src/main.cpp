#include "shell/inc/main.h"

#include <utility>
#include <string>
#include <vector>
#include <SDL2/SDL_events.h>
#include "core/inc/commands.h"
#include "core/inc/window.h"
#include "ent/inc/bullets.h"
#include "ent/inc/control.h"
#include "geo/inc/phys.h"
#include "hacc/inc/files.h"
#include "hacc/inc/haccable_standard.h"
#include "snd/inc/audio.h"
#include "util/inc/debug.h"
#include "util/inc/integration.h"
#include "vis/inc/common.h"

using namespace hacc;
using namespace core;

File main_file = File("shell/main.hacc");
File current_state = File("shell/initial-state.hacc");

static std::string state_arg;

struct Hotkeys : core::Listener {
    std::vector<std::pair<ent::Mapping, core::Command>> hotkeys;
    bool Listener_event (SDL_Event* event) override {
        if (event->type == SDL_KEYDOWN && !event->key.repeat) {
            for (auto& hk : hotkeys) {
                if (hk.first.match(event)) {
                    hk.second();
                    return true;
                }
            }
        }
        return false;
    }
    int Listener_trap_cursor () override { return -1; }
    void finish () { if (!active) activate(); }
};

 // Global game settings.  Eventually, all modules with configurable settings
 //  are expected to reserve a member of this struct.
struct Game {
    bool paused = false;
    core::Command on_start;
    core::Command on_exit;
};
static Game* game = NULL;

void step () {
    if (!game->paused) {
        ent::run_minds();
        geo::space.run_before();
        ent::update_bullets();
        geo::space.run_simulation();
        geo::space.run_after();
        ent::run_minds_after();
    }
}

int main (int argc, char** argv) {
    util::set_logging("file", true);
    hacc::set_file_logger([](std::string s){ util::log("file", s); });
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-no-sound") == 0) {
            snd::use_audio = false;
        }
        else if (argv[i][0] == '-') {
            fprintf(stderr, "Unrecognized command line option: %s\n", argv[i]);
            exit(1);
        }
        else {
            state_arg = util::rel2abs(argv[i]);
        }
    }

    auto here = util::my_dir(argc, argv);
    util::chdir(here + "/modules");

     // Because OpenGL functions won't work until there's a window,
     //  we need to open one ASAP.
    new Window;
    window->width = 640;
    window->height = 480;
    window->step = step;
    window->render = vis::render;
     // Run
    window->open();
    geo::space.start();
    game = main_file.attr("game");
    game->on_start();
    if (FILE* f = fopen("../save/local_config.hacc", "r")) {
        fclose(f);
        core::Command* cmd = File("../save/local_config.hacc").data();
        (*cmd)();
    }
    if (!game->paused) {
        snd::start();
    }
    window->start();
     // After window closes
    if (game->on_exit)
        game->on_exit();
    fprintf(stderr, "Quit successfully\n");
}

HACCABLE(Game) {
    name("Game");
    attr("paused", member(&Game::paused).optional());
    attr("on_start", member(&Game::on_start));
    attr("on_exit", member(&Game::on_exit).optional());
}
HACCABLE(Hotkeys) {
    name("Hotkeys");
    delegate(member(&Hotkeys::hotkeys));
    finish([](Hotkeys& v){ v.finish(); });
}

void pause () {
    game->paused = !game->paused;
    if (game->paused) {
        snd::stop();
    }
    else {
        snd::start();
    }
}
core::New_Command _pause_cmd ("pause", "Toggle whether game activity occurs.", 0, pause);

void _lst (std::string s) {
    if (s.empty()) {
        reload(current_state);
    }
    else {
        unload(current_state);
        current_state = File(s);
        load(current_state);
    }
}
core::New_Command _lst_cmd ("lst", "Load a state (throwing away current state), by default reload same state.", 0, _lst);

void _lst_arg (std::string s) {
    if (!state_arg.empty())
        _lst(state_arg);
    else
        _lst(s);
}

core::New_Command _lst_arg_cmd ("lst_arg", "Load the command-line argument state with this as default.", 0, _lst_arg);

void _sst (std::string s) {
    current_state.rename(s);
    save(current_state);
}

core::New_Command _sst_cmd ("sst", "Save the current state to a file.", 1, _sst);
