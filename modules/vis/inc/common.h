#ifndef HAVE_VIS_COMMON_H
#define HAVE_VIS_COMMON_H

#include "core/inc/window.h"
#include "util/inc/geometry.h"
#include "util/inc/organization.h"

 // The vis namespace handles all rendering (though not all opengl
 //  integration; some of that's in core).

namespace vis {
    using namespace util;

     // This can be called automatically, but it's better to do it manually,
     //  to avoid first-frame lag.
    void init ();

     // Do it.
    void render ();
    template <class C>
    struct Drawn : util::Link<Drawn<C>> {
        virtual void Drawn_draw (C) = 0;

        explicit Drawn (bool visible) { if (visible) link(C::items); }
        void appear () { util::Link<Drawn<C>>::link(C::items); }
        void disappear () { util::Link<Drawn<C>>::unlink(); }
        bool visible () { return util::Link<Drawn<C>>::linked(); }

        Drawn () { }
        virtual ~Drawn () { }
    };

     // These are the layers you can render to.

     // Coordinates: world
     // Input: materials with depth
     // Blend: no
     // Depth: yes
    struct Map { static util::Links<Drawn<Map>> items; };

     // Coordinates: world
     // Input: materials without depth
     // Blend: no
     // Depth: checked but not set
    struct Sprites { static util::Links<Drawn<Sprites>> items; };

     // Coordinates: world
     // Input: none (light values set through uniform)
     // Blend: additive, no alpha
     // Depth: no
    struct Lights { static util::Links<Drawn<Lights>> items; };

     // Coordinates: world
     // Input: color with alpha
     // Blend: alpha blending
     // Depth: no
    struct Overlay { static util::Links<Drawn<Overlay>> items; };

     // Coordinates: camera
     // Input: color with alpha
     // Blend: alpha blending
     // Depth: no
    struct Hud { static util::Links<Drawn<Hud>> items; };

     // Coordinates: dev (strictly 16 window pixels per unit)
     // Input: color with alpha
     // Blend: alpha blending
     // Depth: no
    struct Dev { static util::Links<Drawn<Dev>> items; };

     // There can be multiple cameras, though only one is used at once
     // This is not a haccable item.
    struct Camera {
        Vec pos = Vec(10, 7.5);
        Vec size = Vec(20, 15);
        bool active = false;
        Camera* prev = NULL;

        void activate ();
        void deactivate ();
        Camera () { }
        ~Camera () { deactivate(); }

         // USAGE
        Vec window_to_world (int x, int y) {
            using namespace core;
            return pos - size/2 + Vec(
                x * size.x / window->width,
                (window->height - y) * size.y / window->height
            );
        }
        Vec window_motion_to_world (int x, int y) {
            return Vec(x, -y)*PX;
        }
        Vec window_to_hud (int x, int y) {
            using namespace core;
            return Vec(
                x * size.x / window->width,
                (window->height - y) * size.y / window->height
            );
        }
        Vec window_to_dev (int x, int y) {
            return Vec(x, core::window->height - y)*PX;
        }
        template <class C>
        Vec window_to_layer (int, int);
    };
    extern Camera* camera;

    template <>
    inline Vec Camera::window_to_layer<vis::Map> (int x, int y) {
        return window_to_world(x, y);
    }
    template <>
    inline Vec Camera::window_to_layer<vis::Sprites> (int x, int y) {
        return window_to_world(x, y);
    }
    template <>
    inline Vec Camera::window_to_layer<vis::Overlay> (int x, int y) {
        return window_to_world(x, y);
    }
    template <>
    inline Vec Camera::window_to_layer<vis::Hud> (int x, int y) {
        return window_to_hud(x, y);
    }
    template <>
    inline Vec Camera::window_to_layer<vis::Dev> (int x, int y) {
        return window_to_dev(x, y);
    }

}

#endif
