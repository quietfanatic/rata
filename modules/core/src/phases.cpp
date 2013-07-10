
#include <stdlib.h>
#include "../inc/phases.h"
#include "../inc/commands.h"
#include "../../hacc/inc/everything.h"

namespace core {

    INIT_SAFE(std::vector<Phase*>, all_phases)
    INIT_SAFE(std::vector<Layer*>, all_layers)

} using namespace core;

HCB_BEGIN(Phase*)
    name("core::Phase*");
    hacc::hacc_pointer_by_member(&Phase::name, all_phases());
HCB_END(Phase*)

HCB_BEGIN(Layer*)
    name("core::Layer*");
    hacc::hacc_pointer_by_member(&Layer::name, all_layers());
HCB_END(Layer*)

struct Allow_Command : CommandData {
    Phase* phase = NULL;
    void operator () () {
        if (phase)
            phase->on = true;
        else {
            print_to_console("Available phases are:\n");
            for (auto p : all_phases()) {
                std::string name = p->name.empty() ? "<anonymous>" : p->name;
                print_to_console("\t" + name + " \"" + p->order + "\" " + (p->on ? "true" : "false") + "\n");
            }
        }
    }
};
HCB_BEGIN(Allow_Command)
    new_command<Allow_Command>("allow", "Turn a Phase on (shows available Phases with no argument)");
    elem(member(&Allow_Command::phase).optional());
HCB_END(Allow_Command)

struct Disallow_Command : CommandData {
    Phase* phase;
    void operator () () {
        if (phase) phase->on = false;
    }
};
HCB_BEGIN(Disallow_Command)
    new_command<Disallow_Command>("disallow", "Turn a Phase off");
    elem(member(&Disallow_Command::phase));
HCB_END(Disallow_Command)

struct Show_Command : CommandData {
    Layer* layer = NULL;
    void operator () () {
        if (layer)
            layer->on = true;
        else {
            print_to_console("Available layers are:\n");
            for (auto l : all_layers()) {
                std::string name = l->name.empty() ? "<anonymous>" : l->name;
                print_to_console("\t" + name + " \"" + l->order + "\" " + (l->on ? "true" : "false") + "\n");
            }
        }
    }
};
HCB_BEGIN(Show_Command)
    new_command<Show_Command>("show", "Turn a Layer on (shows the available Layers with no argument)");
    elem(member(&Show_Command::layer).optional());
HCB_END(Show_Command)

struct Hide_Command : CommandData {
    Layer* layer;
    void operator () () { if (layer) layer->on = false; }
};
HCB_BEGIN(Hide_Command)
    new_command<Hide_Command>("hide", "Turn a Layer off");
    elem(member(&Hide_Command::layer));
HCB_END(Hide_Command)


