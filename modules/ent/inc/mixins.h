#ifndef HAVE_ENT_MIXINS_H
#define HAVE_ENT_MIXINS_H

#include "ent/inc/control.h"
#include "geo/inc/phys.h"
#include "geo/inc/rooms.h"
#include "geo/inc/vision.h"
#include "vis/inc/common.h"

namespace ent {

    template <class Layer, class Def = geo::Object_Def>
    struct ROD : geo::Resident, geo::Object, vis::Drawn<Layer> {
        Def* get_def () { return static_cast<Def*>(def); }
        void Resident_emerge () override { materialize(); vis::Drawn<Layer>::appear(); }
        void Resident_reclude () override { vis::Drawn<Layer>::disappear(); dematerialize(); }
        void finish () { Object::finish(); Resident::finish(); }
    };

    struct Damagable {
        virtual int32 Damagable_life () = 0;
        virtual int32 Damagable_max_life () = 0;
        virtual void Damagable_damage (int32) = 0;
    };

    struct Agent_Def : geo::Object_Def {
        util::Vec focus_offset {0, 0};
        int32 max_life = 144;
    };

    template <class Layer, class Def = Agent_Def>
    struct Agent : ROD<Layer, Def>, Controllable, Damagable {
        int32 life = -1;
        int32 Damagable_life () override { return life; }
        int32 Damagable_max_life () override { return ROD<Layer, Def>::get_def()->max_life; }
        void Damagable_damage (int32 d) { life -= d; }

        uint32 buttons = 0;
        util::Vec focus {2, 0};
        util::Vec vision_pos;
        geo::Vision vision;
        void Controllable_buttons (uint32 bits) override {
            if (bits) ROD<Layer, Def>::b2body->SetAwake(true);
            buttons = bits;
        }
        void Controllable_move_focus (util::Vec diff) override {
            focus = constrain(Rect(-18, -13, 18, 13), focus + diff);
        }
        util::Vec Controllable_get_focus () override {
            return focus + get_pos() + ROD<Layer, Def>::get_def()->focus_offset;
        }
        util::Vec Controllable_get_vision_pos () override {
            return vision_pos;
        }
        geo::Room* Controllable_get_room () override {
            return ROD<Layer, Def>::room;
        }
        void finish () {
            ROD<Layer, Def>::finish();
            if (life < 0)
                life = ROD<Layer, Def>::get_def()->max_life;
        }
        void update_vision () {
            auto def = ROD<Layer, Def>::get_def();
            util::Vec origin = get_pos() + def->focus_offset;
            vision.attend(origin + Rect(-1, -1, 1, 1), 1000000);
            util::Vec focus_world = focus + origin;
            vision_pos = vision.look(origin, &focus_world, !!controller);
            focus = focus_world - origin;
        }
        void Object_after_move () override {
            update_vision();
        }
    };

}

 // This is provided, but you can skip directly to each of the bases instead.
HACCABLE_TEMPLATE(<class Layer HCB_COMMA class Def>, ent::ROD<Layer HCB_COMMA Def>) {
    name([](){
        return "ent::ROD<" + hacc::Type::CppType<Layer>().name()
             + ", " + hacc::Type::CppType<Def>().name() + ">";
    });
    attr("Resident", hcb::template base<geo::Resident>().collapse());
    attr("Object", hcb::template base<geo::Object>().collapse());
    finish(&ent::ROD<Layer, Def>::finish);
}
HACCABLE_TEMPLATE(<class Layer HCB_COMMA class Def>, ent::Agent<Layer HCB_COMMA Def>) {
    name([](){
        return "ent::Agent<" + hacc::Type::CppType<Layer>().name()
             + ", " + hacc::Type::CppType<Def>().name() + ">";
    });
    attr("ROD", hcb::template base<ent::ROD<Layer, Def>>().collapse());
    attr("focus", member(&ent::Agent<Layer, Def>::focus).optional());
    attr("life", member(&ent::Agent<Layer, Def>::life).optional());
    finish(&ent::Agent<Layer, Def>::finish);
}

#endif
