
#include "../../hacc/inc/everything.h"
#include "../inc/mixins.h"
#include "../../vis/inc/images.h"
#include "../../util/inc/geometry.h"

using namespace util;

namespace ent {

    struct Crate : ROD<vis::Sprites> {
        static phys::BodyDef* bdf;
        static vis::Texture* texture;

        void Drawn_draw (vis::Sprites) override {
            vis::draw_texture(texture, pos() + Rect(0.5, 0, 0.5, 1));
        }

        Crate () {
            static bool initted = false;
            if (!initted) {
                initted = true;
                bdf = hacc::File("ent/res/various.hacc").data().attr("crate_bdf");
                texture = hacc::File("ent/res/various.hacc").data().attr("stuff_img").attr("crate");
                hacc::manage(bdf);
                hacc::manage(texture);
            }
        }
        void finish () {
            set_def(bdf);
        }
    };
    phys::BodyDef* Crate::bdf = NULL;
    vis::Texture* Crate::texture = NULL;

} using namespace ent;

HACCABLE(Crate) {
    name("ent::Crate");
    attr("ROD", base<ROD<vis::Sprites>>().optional());
    finish(&Crate::finish);
}

static phys::BodyDef*& boundary_bdf () {
    static phys::BodyDef* boundary_bdf = hacc::File("ent/res/various.hacc").data().attr("boundary_bdf");
    hacc::manage(&boundary_bdf);
    return boundary_bdf;
}
struct Boundary : phys::Object {
    Boundary () : Object() { boundary_bdf(); }
    void finish () {
        set_def(boundary_bdf());
        materialize();
    }
};

HACCABLE(Boundary) {
    name("ent::Boundary");
    attr("Object", base<phys::Object>());
    finish([](Boundary& b){ b.finish(); });
}

