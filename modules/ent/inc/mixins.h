#ifndef HAVE_ENT_MIXINS_H
#define HAVE_ENT_MIXINS_H

#include "../../geo/inc/rooms.h"
#include "../../phys/inc/phys.h"
#include "../../vis/inc/common.h"

namespace ent {

    template <class Layer>
    struct ROD : geo::Resident, phys::Object, vis::Drawn<Layer> {
        Vec Resident_get_pos () override { return Object::pos(); }
        void Resident_set_pos (Vec p) override { Object::set_pos(p); }
        void Resident_emerge () override { materialize(); vis::Drawn<Layer>::appear(); }
        void Resident_reclude () override { vis::Drawn<Layer>::disappear(); dematerialize(); }
    };

}

 // This is provided, but you can skip directly to each of the bases instead.
HACCABLE_TEMPLATE(<class Layer>, ent::ROD<Layer>) {
    name([](){ return "ent::ROD<" + hacc::Type::CppType<Layer>().name() + ">"; });
    attr("Resident", hcb::template base<geo::Resident>().optional());
    attr("Object", hcb::template base<phys::Object>().optional());
}

#endif
