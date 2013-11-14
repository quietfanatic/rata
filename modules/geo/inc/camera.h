#ifndef HAVE_GEO_CAMERA_H
#define HAVE_GEO_CAMERA_H

#include "../../core/inc/window.h"
#include "../../util/inc/geometry.h"

 // TODO: port the constraint-satisfaction camera control from the old branch.

namespace geo {
    using namespace util;

     // This is actually an interface
    struct Camera {
        virtual Vec Camera_pos () { return Vec(10, 7.5); }
        virtual Vec Camera_size () { return Vec(20, 15); }
        virtual void Camera_update () { }
        virtual ~Camera () { deactivate(); }

        Camera (bool active = false) { activate(); }
        Camera* prev = NULL;
        void activate ();
        void deactivate ();
         // USAGE
        Vec window_to_world (int x, int y) {
            using namespace core;
            Vec pos = Camera_pos();
            Vec size = Camera_size();
            return pos - size/2 + Vec(
                x * size.x / window->width,
                (window->height - y) * size.y / window->height
            );
        }
        Vec window_motion_to_world (int x, int y) {
            return Vec(x, -y)*PX;
        }
    };
    extern Camera* camera;

    struct Default_Camera : Camera {
        Vec pos = Vec(10, 7.5);
        Vec Camera_pos () override { return pos; }
        void Camera_update () { }
        void finish () { activate(); }
        Default_Camera (bool active = true) : Camera(active) { }
    };
    Default_Camera& default_camera ();

    struct Free_Camera : Camera {
        Vec pos = Vec(10, 7.5);
        Vec size = Vec(20, 15);
        Vec Camera_pos () override { return pos; }
        Vec Camera_size () override { return size; }
        void Camera_update () override;
    };

}

#endif
