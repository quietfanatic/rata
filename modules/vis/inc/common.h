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

    struct Material {
        uint32 ambient;
        uint32 diffuse;
        uint32 radiant;
    };

    struct Materials {
        std::vector<Material> items;
        GLuint tex = 0;
        void update ();
        ~Materials ();
    };

    void set_materials (Materials*);

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

     // Mainly for lighting calculations; needs to be able to go above 1
    struct RGBf {
        float r;
        float g;
        float b;
        RGBf& operator += (const RGBf& o) {
            r += o.r;
            g += o.g;
            b += o.b;
            return *this;
        }
    };

}

#endif
