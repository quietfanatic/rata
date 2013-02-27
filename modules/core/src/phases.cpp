
#include <stdlib.h>
#include "../inc/phases.h"
#include "../inc/input.h"
#include "../inc/commands.h"
#include "../../hacc/inc/everything.h"

namespace core {

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
    void operator () () {
        if (phase)
            phase->on = true;
        else {
            print_to_console("Available phases are:\n");
            for (auto p : game_phases()) {
                std::string name = p->name.empty() ? "<anonymous>" : p->name;
                print_to_console("\t" + name + " \"" + p->order + "\" " + (p->on ? "true" : "false"));
            }
        }
    }
};
HCB_BEGIN(Allow_Command)
    base<Command>("allow");
    elem(member(&Allow_Command::phase, def((Phase*)NULL)));
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
    void operator () () {
        if (layer)
            layer->on = true;
        else {
            print_to_console("Available layers are:\n");
            for (auto l : draw_layers()) {
                std::string name = l->name.empty() ? "<anonymous>" : l->name;
                print_to_console("\t" + name + " \"" + l->order + "\" " + (l->on ? "true" : "false"));
            }
        }
    }
};
HCB_BEGIN(Show_Command)
    base<Command>("show");
    elem(member(&Show_Command::layer, def((Layer*)NULL)));
HCB_END(Show_Command)

struct Hide_Command : Command {
    Layer* layer;
    void operator () () { if (layer) layer->on = false; }
};
HCB_BEGIN(Hide_Command)
    base<Command>("hide");
    elem(member(&Hide_Command::layer));
HCB_END(Hide_Command)


