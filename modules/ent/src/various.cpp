
#include <vector>
#include "ent/inc/mixins.h"
#include "hacc/inc/everything.h"
#include "phys/inc/ground.h"
#include "util/inc/geometry.h"
#include "vis/inc/images.h"
#include "vis/inc/light.h"

using namespace util;
using namespace vis;
using namespace phys;

namespace ent {

    struct Inert_Def : Object_Def {
        Texture* texture;
        Rect boundary;
    };

    struct Ext_Def : Object_Def {
        vis::Texture* texture;
        vis::Layout* layout;
    };

    struct Inert : ROD<Sprites, Inert_Def> {
        void Drawn_draw (Sprites) override {
            auto def = get_def();
            draw_texture(def->texture, pos() + def->boundary);
        }
    };

    struct Light : geo::Resident, Drawn<Lights> {
        Vec pos;
        std::vector<Vec> vertices;
        RGBf ambient;
        RGBf diffuse;
        RGBf radiant;
        void Drawn_draw (Lights) override {
            light_offset(pos);
            set_ambient(ambient);
            set_diffuse(diffuse);
            set_radiant(radiant);
            draw_light(vertices.size(), vertices.data());
        }
        Vec Resident_get_pos () override { return pos; }
        void Resident_set_pos (Vec p) override { pos = p; }
        void Resident_emerge () override { appear(); }
        void Resident_reclude () override { disappear(); }
        size_t Resident_n_pts () override { return vertices.size(); }
        Vec Resident_get_pt (size_t i) override { return vertices.at(i); }
        void Resident_set_pt (size_t i, Vec v) override {
            vertices.at(i) = v;
        }
    };

    struct Patroller : ROD<Sprites, Ext_Def>, phys::Grounded {
        enum Frame {
            UP,
            DOWN
        };
        void Drawn_draw (Sprites) override {
            auto def = get_def();
            draw_frame(&def->layout->frames[UP], def->texture, pos());
        }
    };

} using namespace ent;

HACCABLE(Inert_Def) {
    name("ent::Inert_Def");
    attr("Object_Def", base<Object_Def>().collapse());
    attr("texture", member(&Inert_Def::texture));
    attr("boundary", member(&Inert_Def::boundary));
}

HACCABLE(Ext_Def) {
    name("ent::Ext_Def");
    attr("Object_Def", base<Object_Def>().collapse());
    attr("texture", member(&Ext_Def::texture));
    attr("layout", member(&Ext_Def::layout));
}

HACCABLE(Inert) {
    name("ent::Inert");
    attr("ROD", base<ROD<Sprites, Inert_Def>>().collapse());
}

HACCABLE(Light) {
    name("ent::Light");
    attr("Resident", base<geo::Resident>().collapse());
    attr("pos", member(&Light::pos).optional());
    attr("vertices", member(&Light::vertices).optional());
    attr("ambient", member(&Light::ambient).optional());
    attr("diffuse", member(&Light::diffuse).optional());
    attr("radiant", member(&Light::radiant).optional());
}

HACCABLE(Patroller) {
    name("ent::Patroller");
    attr("ROD", base<ROD<Sprites, Ext_Def>>().collapse());
}

