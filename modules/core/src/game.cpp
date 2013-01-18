
#include <stdlib.h>
#include "GL/glfw.h"

#include "../inc/game.h"
#include "../inc/input.h"
#include "../inc/commands.h"
#include "../../hacc/inc/everything.h"

namespace core {

    void all_phases (void (PhaseLayer::* method )()) {
        for (PhaseLayer* p : game_phases())
            (p->*method)();
    }
    void all_layers (void (PhaseLayer::* method )()) {
        for (PhaseLayer* p : draw_layers())
            (p->*method)();
    }
    void all_phaselayers (void (PhaseLayer::* method )()) {
        all_phases(method);
        all_layers(method);
    }

    void quit_game () {
        glfwTerminate();
        exit(0);
    }
    void set_video (uint scale) {
        glfwOpenWindow(320*scale, 240*scale, 8, 8, 8, 0, 0, 0, GLFW_WINDOW);
    }
    
    static bool initialized = false;

    void init () {
        if (initialized) return;
        initialized = true;
        glfwInit();
        set_video(2);
        all_phaselayers(&PhaseLayer::init);
    }

    void start () {
        try {
            init();
            all_phaselayers(&PhaseLayer::start);
            for (;;) {
                all_phases(&PhaseLayer::run_if_on);
                all_layers(&PhaseLayer::run_if_on);
                glfwSwapBuffers();
                glfwSleep(1/60.0);
            }
        } catch (std::exception& e) {
            fprintf(stderr, "Game was aborted due to an exception: %s\n", e.what());
        }
    }

    void stop () {
        all_phaselayers(&PhaseLayer::stop);
    }

    PhaseLayer::PhaseLayer (std::vector<PhaseLayer*>& type, std::string order, std::string name, bool on) :
        order(order), name(name), on(on)
    {
        for (auto i = type.begin(); i != type.end(); i++) {
            if (order < (*i)->order) {
                type.insert(i, this);
                return;
            }
        }
        type.push_back(this);
    }

    std::vector<PhaseLayer*>& game_phases () { static std::vector<PhaseLayer*> r; return r; }
    std::vector<PhaseLayer*>& draw_layers () { static std::vector<PhaseLayer*> r; return r; }

}

using namespace core;

HCB_BEGIN(Phase)
    type_name("core::Phase");
    get_id([](const Phase& p){ return p.name; });
    find_by_id([](std::string id){
        for (auto& pl : game_phases())
            if (pl->name == id)
                return (Phase*)pl;
        return (Phase*)NULL;
    });
HCB_END(Phase)

struct Allow_Command : Command {
    Phase* phase;
    void operator () () { if (phase) phase->on = true; }
};
HCB_BEGIN(Allow_Command)
    base<Command>("allow");
    elem(member(&Allow_Command::phase));
HCB_END(Allow_Command)

struct Disallow_Command : Command {
    Phase* phase;
    void operator () () { if (phase) phase->on = false; }
};
HCB_BEGIN(Disallow_Command)
    base<Command>("disallow");
    elem(member(&Disallow_Command::phase));
HCB_END(Disallow_Command)


HCB_BEGIN(Layer)
    type_name("core::Layer");
    get_id([](const Layer& p){ return p.name; });
    find_by_id([](std::string id){
        for (auto& pl : draw_layers())
            if (pl->name == id)
                return (Layer*)pl;
        return (Layer*)NULL;
    });
HCB_END(Layer)

struct Show_Command : Command {
    Layer* layer;
    void operator () () { if (layer) layer->on = true; }
};
HCB_BEGIN(Show_Command)
    base<Command>("show");
    elem(member(&Show_Command::layer));
HCB_END(Show_Command)

struct Hide_Command : Command {
    Layer* layer;
    void operator () () { if (layer) layer->on = false; }
};
HCB_BEGIN(Hide_Command)
    base<Command>("hide");
    elem(member(&Hide_Command::layer));
HCB_END(Hide_Command)


