#include "ent/inc/bullets.h"

#include "hacc/inc/haccable_standard.h"
#include "phys/inc/phys.h"
#include "vis/inc/color.h"

namespace ent {
    using namespace vis;

    Links<Bullet> bullets;

    void Bullet::update () {
    }
    void Bullet::Drawn_draw (Overlay) {
        uint n_pts = 0;
        for (uint i = 0; i < MAX_PTS; i++) {
            if (!defined(pts[i])) {
                n_pts = i;
                break;
            }
        }
        draw_color(0xffffff7f);
        color_offset(Vec(0, 0));
        draw_chain(n_pts, pts);
    }

    void update_bullets () {
         // Delete-safe loop
        for (auto bp = bullets.begin(); bp != bullets.end();) {
            auto b = bp++;
            b->update();
        }
    }

} using namespace ent;

HACCABLE(Bullet) {
    name("ent::Bullet");
    attr("owner", member(&Bullet::owner));
    attr("pts", member(&Bullet::pts));
    attr("vel", member(&Bullet::vel));
    finish(&Bullet::finish);
}
