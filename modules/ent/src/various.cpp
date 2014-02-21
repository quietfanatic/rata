
#include "../../hacc/inc/everything.h"
#include "../inc/mixins.h"
#include "../../vis/inc/images.h"
#include "../../vis/inc/light.h"
#include "../../util/inc/geometry.h"

using namespace util;
using namespace vis;

namespace ent {

    struct Crate : ROD<Sprites> {
        static phys::BodyDef* bdf;
        static Texture* texture;

        void Drawn_draw (Sprites) override {
            draw_texture(texture, pos() + Rect(-0.5, 0, 0.5, 1));
        }

        Crate () {
            static bool initted = false;
            if (!initted) {
                initted = true;
                bdf = hacc::File("ent/res/various.hacc").data().attr("crate_bdf");
                texture = hacc::File("ent/res/various.hacc").data().attr("stuff_img").attr("crate");
                hacc::manage(&bdf);
                hacc::manage(&texture);
            }
        }
        void finish () {
            Object::set_bdf(bdf);
            Resident::finish();
        }
    };
    phys::BodyDef* Crate::bdf = NULL;
    Texture* Crate::texture = NULL;

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
    };

} using namespace ent;

HACCABLE(Crate) {
    name("ent::Crate");
    attr("ROD", base<ROD<Sprites>>().optional());
    finish(&Crate::finish);
}

HACCABLE(Light) {
    name("ent::Light");
    attr("Resident", base<geo::Resident>().optional());
    attr("pos", member(&Light::pos).optional());
    attr("vertices", member(&Light::vertices).optional());
    attr("ambient", member(&Light::ambient).optional());
    attr("diffuse", member(&Light::diffuse).optional());
    attr("radiant", member(&Light::radiant).optional());
}

static phys::BodyDef*& boundary_bdf () {
    static phys::BodyDef* boundary_bdf = hacc::File("ent/res/various.hacc").data().attr("boundary_bdf");
    hacc::manage(&boundary_bdf);
    return boundary_bdf;
}
struct Boundary : phys::Object {
    Boundary () : Object() { boundary_bdf(); }
    void finish () {
        set_bdf(boundary_bdf());
        materialize();
    }
};

HACCABLE(Boundary) {
    name("ent::Boundary");
    attr("Object", base<phys::Object>());
    finish([](Boundary& b){ b.finish(); });
}

