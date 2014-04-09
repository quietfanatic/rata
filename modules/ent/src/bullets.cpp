#include "ent/inc/bullets.h"

#include "hacc/inc/haccable_standard.h"
#include "phys/inc/phys.h"
#include "util/inc/debug.h"
#include "vis/inc/color.h"

namespace ent {
    using namespace vis;

    Links<Bullet> bullets;

    void Bullet::update () {
        pts[1] = Vec(NAN, NAN);
        Vec new_pos = pts[0] + vel;
        b2Fixture* old_struck = NULL;
        try_bounce: {
            float earliest_fraction = 1;
            b2Fixture* struck = NULL;
            Vec pos;
            Vec normal;
            phys::space.ray_cast(pts[0], new_pos, [&](b2Fixture* fix, const Vec& p, const Vec& n, float fraction)->float{
                if (fix->GetBody() == owner->b2body) return -1;
                 // Prevent infinite detection
                if (fix == old_struck) return -1;
                 // TODO check filter
                if (fraction < earliest_fraction) {
                    struck = fix;
                    pos = p;
                    normal = n;
                }
                return fraction;
            });
            if (struck) {
                old_struck = struck;
                 // TODO check ricochet angle
                pts[3] = pts[2];
                pts[2] = pts[1];
                pts[1] = pts[0];
                pts[0] = pos;
                 // This is how you bounce
                float vel_perp = dot(vel, normal);
                vel -= 2 * vel_perp * normal;
                new_pos = pts[0] + vel * (1 - earliest_fraction);
                 // We've ended up with a more than 100% elastic collision but oh well
                struck->GetBody()->ApplyLinearImpulse(vel_perp * normal, pos, true);
                log("bullet", "Bullet velocity perpendicular: %f", vel_perp);
                if (vel_perp > -0.8) {
                     // TODO play ricochet sound
                    goto try_bounce;
                }
                else {
                     // TODO delete self
                }
            }
        }
        pts[3] = pts[2];
        pts[2] = pts[1];
        pts[1] = pts[0];
        pts[0] = new_pos;
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
