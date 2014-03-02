#ifndef HAVE_VIS_COMMON_H
#define HAVE_VIS_COMMON_H

#include "util/inc/geometry.h"
#include "util/inc/organization.h"

 // The vis namespace handles all rendering (though not all opengl
 //  integration; some of that's in core).

namespace vis {

     // Setting this alters rendering until the Hud step.
    extern util::Vec camera_pos;
    extern util::Vec camera_size;

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

     // For primarily internal use
    extern util::Vec global_camera_pos;
    extern util::Vec global_camera_size;

}

#endif
