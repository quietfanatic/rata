#ifndef HAVE_ENT_MIXINS_H
#define HAVE_ENT_MIXINS_H

#include "geo/inc/rooms.h"
#include "phys/inc/phys.h"
#include "vis/inc/common.h"

namespace ent {

    template <class Layer, class Def = phys::Object_Def>
    struct ROD : geo::Resident, phys::Object, vis::Drawn<Layer> {
        Def* get_def () { return static_cast<Def*>(def); }
        void Resident_emerge () override { materialize(); vis::Drawn<Layer>::appear(); }
        void Resident_reclude () override { vis::Drawn<Layer>::disappear(); dematerialize(); }
        void finish () { Object::finish(); Resident::finish(); }
    };

     // Must be in the state document
    struct Damagable {
        virtual void Damagable_damage (int32) = 0;
    };

}

 // This is provided, but you can skip directly to each of the bases instead.
HACCABLE_TEMPLATE(<class Layer HCB_COMMA class Def>, ent::ROD<Layer HCB_COMMA Def>) {
    name([](){
        return "ent::ROD<" + hacc::Type::CppType<Layer>().name()
             + ", " + hacc::Type::CppType<Def>().name() + ">";
    });
    attr("Resident", hcb::template base<geo::Resident>().collapse());
    attr("Object", hcb::template base<phys::Object>().collapse());
    finish(&ent::ROD<Layer, Def>::finish);
}

#endif
