
#include <stdlib.h>
#include "../inc/phases.h"
#include "../inc/input.h"
#include "../inc/commands.h"
#include "../../hacc/inc/everything.h"

namespace core {

    std::vector<Phase*> all_phases;
    std::vector<Layer*> all_layers;

} using namespace core;

HCB_BEGIN(Phase*)
    name("core::Phase*");
    hacc::hacc_pointer_by_member(&Phase::name, all_phases);
HCB_END(Phase*)

HCB_BEGIN(Layer*)
    name("core::Layer*");
    hacc::hacc_pointer_by_member(&Layer::name, all_layers);
HCB_END(Layer*)

