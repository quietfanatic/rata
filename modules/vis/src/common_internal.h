#ifndef HAVE_VIS_COMMON_INTERNAL_H
#define HAVE_VIS_COMMON_INTERNAL_H
#include "vis/inc/common.h"

#include "core/inc/opengl.h"

namespace vis {

     // Programs that inherit from this will be well-behaved regarding
     //  the game's camera.  The program must have two uniforms: camera_pos
     //   and camera_size, both vec2s.
    struct Cameraed_Program : core::Program {
        GLint camera_pos = 0;
        GLint camera_size = 0;
        util::Vec old_camera_pos = util::Vec(0, 0);
        util::Vec old_camera_size = util::Vec(0, 0);
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
