#include "ent/inc/mixins.h"

HACCABLE(ent::Agent_Def) {
    name("ent::Agent_Def");
    attr("Object_Def", base<phys::Object_Def>().collapse());
    attr("focus_offset", member(&ent::Agent_Def::focus_offset).optional());
}
