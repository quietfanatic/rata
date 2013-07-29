#ifndef HAVE_VIS_COMMON_H
#define HAVE_VIS_COMMON_H

#include "../../util/inc/geometry.h"
#include "../../util/inc/organization.h"
#include "../../core/inc/opengl.h"

 // The vis namespace handles all rendering (though not all opengl
 //  integration; some of that's in core).

namespace vis {
    using namespace util;

     // Setting this alters rendering until the Hud step.
    extern Vec camera_pos;

     // This can be called automatically, but it's better to do it manually,
     //  to avoid first-frame lag.
    void init ();

    struct Settings {
         // The canonical camera size is (10, 7.5), but it can
         //  be expanded for development purposes.
        Vec camera_size = Vec(10, 7.5);
        Settings ();
        ~Settings ();
    };
    extern Settings* settings;

     // Do it.
    void render ();

    template <class C>
    struct Drawn : Link<Drawn<C>> {
        virtual void Drawn_draw (C) = 0;
        Drawn () { }
        Drawn (bool visible) { if (visible) link(C::items); }
        void appear () { Link<Drawn<C>>::link(C::items); }
        void disappear () { Link<Drawn<C>>::unlink(); }
        bool visible () { return Link<Drawn<C>>::linked(); }
    };

     // Uses depth, materials
    struct Map { static Links<Drawn<Map>> items; };
     // Uses depth, materials
    struct Sprites { static Links<Drawn<Sprites>> items; };
     // Alpha blending, no depth or materials
    struct Overlay { static Links<Drawn<Overlay>> items; };
     // Alpha blending, no depth or materials, fixed camera
    struct Hud { static Links<Drawn<Hud>> items; };
     // Ignores camera_size (Full pixel resolution for console and stuff)
     // Sixteen real-pixels per space unit (as opposed to virtual pixels).
    struct Dev { static Links<Drawn<Dev>> items; };

     // For primarily internal use
    extern Vec global_camera_pos;
    extern Vec global_camera_size;

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
