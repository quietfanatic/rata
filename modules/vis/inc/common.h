#ifndef HAVE_VIS_COMMON_H
#define HAVE_VIS_COMMON_H

#include "images.h"
#include "../../util/inc/geometry.h"
#include "../../util/inc/organization.h"
#include "../../core/inc/opengl.h"

 // The vis namespace handles all rendering (though not all opengl
 //  integration; some of that's in core).

namespace vis {
    using namespace util;

     // Setting this alters rendering until the Hud step.
    extern Vec camera_pos;
    extern Vec camera_size;

     // This can be called automatically, but it's better to do it manually,
     //  to avoid first-frame lag.
    void init ();

     // Do it.
    void render ();
    template <class C>
    struct Drawn : Link<Drawn<C>> {
        virtual void Drawn_draw (C) = 0;

        Drawn (bool visible) { if (visible) link(C::items); }
        void appear () { Link<Drawn<C>>::link(C::items); }
        void disappear () { Link<Drawn<C>>::unlink(); }
        bool visible () { return Link<Drawn<C>>::linked(); }

        Drawn () { }
        virtual ~Drawn () { disappear(); }
    };

     // These are the layers you can render to.

     // Coordinates: world
     // Input: materials with depth
     // Blend: no
     // Depth: yes
    struct Map { static Links<Drawn<Map>> items; };

     // Coordinates: world
     // Input: materials without depth
     // Blend: no
     // Depth: checked but not set
    struct Sprites { static Links<Drawn<Sprites>> items; };

     // Coordinates: world
     // Input: none (light values set through uniform)
     // Blend: additive, no alpha
     // Depth: no
    struct Lights { static Links<Drawn<Lights>> items; };

     // Coordinates: world
     // Input: color with alpha
     // Blend: alpha blending
     // Depth: no
    struct Overlay { static Links<Drawn<Overlay>> items; };

     // Coordinates: camera
     // Input: color with alpha
     // Blend: alpha blending
     // Depth: no
    struct Hud { static Links<Drawn<Hud>> items; };

     // Coordinates: dev (strictly 16 window pixels per unit)
     // Input: color with alpha
     // Blend: alpha blending
     // Depth: no
    struct Dev { static Links<Drawn<Dev>> items; };

     // For primarily internal use
    extern Vec global_camera_pos;
    extern Vec global_camera_size;

     // Programs that inherit from this will be well-behaved regarding
     //  the game's camera.  The program must have two uniforms: camera_pos
     //   and camera_size, both vec2s.
    struct Cameraed_Program : core::Program {
        GLint camera_pos = 0;
        GLint camera_size = 0;
        Vec old_camera_pos = Vec(0, 0);
        Vec old_camera_size = Vec(0, 0);
        void Program_begin () override {
            Program::Program_begin();
            if (old_camera_pos != global_camera_pos) {
                glUniform2f(camera_pos, global_camera_pos.x, global_camera_pos.y);
                old_camera_pos = global_camera_pos;
            }
            if (old_camera_size != global_camera_size) {
                glUniform2f(camera_size, global_camera_size.x, global_camera_size.y);
            }
        }
        void finish () {
            Program::link();
            glUseProgram(glid);
            camera_pos = require_uniform("camera_pos");
            camera_size = require_uniform("camera_size");
        }
    };

}

#endif
