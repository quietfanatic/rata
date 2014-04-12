#include "ent/inc/bullets.h"

#include "ent/inc/mixins.h"
#include "geo/inc/phys.h"
#include "hacc/inc/haccable_standard.h"
#include "shell/inc/main.h"
#include "snd/inc/audio.h"
#include "util/inc/debug.h"
#include "vis/inc/color.h"

namespace ent {
    using namespace vis;
    using namespace geo;

    Links<Bullet> bullets;

    static snd::Voice* shoot_voice;
    static snd::Voice* ricochet_voice;

    void Bullet::update () {
        if (done) {
            state_document()->destroy(this);
            return;
        }
        pts[1] = Vec(NAN, NAN);
        Vec new_pos = pts[0] + vel;
        b2Fixture* old_struck = NULL;
        try_bounce: {
            float earliest_fraction = 1;
            b2Fixture* struck = NULL;
            Vec pos;
            Vec normal;
            float vel_perp;
            space.ray_cast(pts[0], new_pos, [&](b2Fixture* fix, const Vec& p, const Vec& n, float fraction)->float{
                if (fix->GetBody() == owner->b2body) return -1;
                 // Prevent infinite detection
                if (fix == old_struck) return -1;
                 // TODO check filter
                if (fraction < earliest_fraction) {
                    float vp = dot(vel, n);
                     // Ignore backwards collisions
                    if (vp > 0) return -1;
                    vel_perp = vp;
                    struck = fix;
                    pos = p;
                    normal = n;
                    return fraction;
                }
                return -1;
            });
            if (struck) {
                old_struck = struck;
                 // TODO check ricochet angle
                pts[3] = pts[2];
                pts[2] = pts[1];
                pts[1] = pts[0];
                pts[0] = pos;
                 // This is how you bounce
                vel -= 2 * vel_perp * normal;
                new_pos = pts[0] + vel * (1 - earliest_fraction);
                 // We've ended up with a more than 100% elastic collision but oh well
                struck->GetBody()->ApplyLinearImpulse(vel_perp * normal, pos, true);
                if (auto d = dynamic_cast<Damagable*>((Object*)struck->GetBody()->GetUserData())) {
                    log("bullet", "%p hitting %p for 48 damage", this, d);
                    d->Damagable_damage(48);
                    done = true;
                    return;
                }
                log("bullet", "Bullet velocity perpendicular: %f", vel_perp);
                if (vel_perp > -0.8) {
                    goto try_bounce;
                }
                else {
                    done = true;
                    return;
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
        static bool initted = false;
        if (!initted) {
            initted = true;
            shoot_voice = hacc::File("world/snd/sounds.hacc").attr("shoot_voice");
            ricochet_voice = hacc::File("world/snd/sounds.hacc").attr("ricochet_voice");
            hacc::manage(&ricochet_voice);
        }
         // Delete-safe loop
        for (auto bp = bullets.begin(); bp != bullets.end();) {
            auto b = bp++;
            b->update();
        }
    }

    void shoot_sound (float volume) {
        shoot_voice->done = false;
        shoot_voice->paused = false;
         // TODO: make this not necessary
        shoot_voice->pos = 5512;
        shoot_voice->volume = volume;
    }

    void ricochet_sound (float volume) {
        ricochet_voice->done = false;
        ricochet_voice->paused = false;
         // TODO: make this not necessary
        ricochet_voice->pos = 5512;
        ricochet_voice->volume = volume;
    }

    Bullet* shoot_bullet (Object* owner, Vec pos, Vec vel) {
        shoot_sound();
        return state_document()->create<Bullet>(owner, pos, vel);
    }

} using namespace ent;

HACCABLE(Bullet) {
    name("ent::Bullet");
    attr("owner", member(&Bullet::owner));
    attr("pts", member(&Bullet::pts));
    attr("vel", member(&Bullet::vel));
    attr("done", member(&Bullet::done));
    finish(&Bullet::finish);
}
