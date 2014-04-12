
#include <vector>
#include "ent/inc/mixins.h"
#include "hacc/inc/everything.h"
#include "shell/inc/main.h"
#include "util/inc/geometry.h"
#include "vis/inc/images.h"
#include "vis/inc/light.h"

using namespace util;
using namespace vis;
using namespace geo;

namespace ent {

    struct Inert_Def : Object_Def {
        Texture* texture = NULL;
        Rect boundary;
        int32 max_life = -1;  // If negative, invincible
    };

    struct Inert : ROD<Sprites, Inert_Def>, Damagable {
        int32 life = -1;
        void finish () {
            if (life < 0) life = get_def()->max_life;
            ROD::finish();
        }
        void Drawn_draw (Sprites) override {
            auto def = get_def();
            draw_texture(def->texture, get_pos() + def->boundary);
        }
        int32 Damagable_life () override { return life; }
        int32 Damagable_max_life () override { return get_def()->max_life; }
        void Damagable_damage (int32 d) override {
            if (life >= 0) {
                life -= d;
                if (life <= 0) {
                    state_document()->destroy(this);
                }
            }
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
        Vec Spatial_get_pos () override { return pos; }
        void Spatial_set_pos (Vec p) override { pos = p; }
        size_t Spatial_n_pts () override { return vertices.size(); }
        Vec Spatial_get_pt (size_t i) override { return vertices.at(i); }
        void Spatial_set_pt (size_t i, Vec v) override {
            vertices.at(i) = v;
        }
        void Resident_emerge () override { appear(); }
        void Resident_reclude () override { disappear(); }
    };

} using namespace ent;

HACCABLE(Inert_Def) {
    name("ent::Inert_Def");
    attr("Object_Def", base<Object_Def>().collapse());
    attr("texture", member(&Inert_Def::texture));
    attr("boundary", member(&Inert_Def::boundary));
    attr("max_life", member(&Inert_Def::max_life).optional());
}

HACCABLE(Inert) {
    name("ent::Inert");
    attr("ROD", base<ROD<Sprites, Inert_Def>>().collapse());
    attr("life", member(&Inert::life).optional());
    finish(&Inert::finish);
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
