
#include "../../hacc/inc/everything.h"
#include "../../phys/inc/phys.h"
#include "../../vis/inc/common.h"
#include "../../vis/inc/images.h"

struct Crate : phys::Object, vis::Drawn<vis::Sprites> {
    static phys::BodyDef* bdf;
    static vis::Texture* texture;
    static vis::Frame* frame;

    void Drawn_draw (vis::Sprites) {
        vis::draw_frame(frame, texture, pos());
    }

    Crate () : Object() {
        static bool initted = false;
        if (!initted) {
            initted = true;
            bdf = hacc::File("ent/res/various.hacc").data().attr("crate_bdf");
            texture = hacc::File("ent/res/various.hacc").data().attr("stuff_img").attr("ALL");
            frame = hacc::File("ent/res/various.hacc").data().attr("stuff_layout").attr("crate");
            hacc::manage(bdf);
            hacc::manage(texture);
            hacc::manage(frame);
        }
    }
    void finish () {
        set_def(bdf);
        materialize();
        appear();
    }
};
phys::BodyDef* Crate::bdf = NULL;
vis::Texture* Crate::texture = NULL;
vis::Frame* Crate::frame = NULL;

HACCABLE(Crate) {
    name("ent::Crate");
    attr("Object", base<phys::Object>());
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

