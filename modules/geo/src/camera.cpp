#include "geo/inc/camera.h"
using namespace core;

namespace geo {

    void Free_Camera::Camera_update () {
         // TODO: do stuff
        if (!window->cursor_trapped) {
            float move_speed = 1/256.0;
            if (window->cursor_x < 64) {
                pos.x += (window->cursor_x - 64) * move_speed;
            }
            else if (window->cursor_x > window->width - 64) {
                pos.x += (window->cursor_x - window->width + 64) * move_speed;
            }
            if (window->cursor_y < 64) {
                pos.y -= (window->cursor_y - 64) * move_speed;
            }
            else if (window->cursor_y > window->height - 64) {
                pos.y -= (window->cursor_y - window->height + 64) * move_speed;
            }
        }
        if (!pos.is_defined())
            pos = Vec(10, 7.5);
    }

    Camera* camera = NULL;
    void Camera::activate () {
        deactivate();
        prev = camera;
        camera = this;
        active = true;
    }
    void Camera::deactivate () {
        if (active) {
            for (Camera** c = &camera; *c; c = &(*c)->prev) {
                if (*c == this) {
                    *c = prev;
                    prev = NULL;
                    return;
                }
            }
        }
        prev = NULL;
        active = false;
    }

    Default_Camera& default_camera () {
        static Default_Camera r (true);
        return r;
    }

    Links<Camera_Bound> camera_bounds;

    void Camera_Bound::finish () {
        if (right) {
            edge = double_tangent(corner, right->corner);
        }
        else {
            edge = Line();
        }
    }
    void Camera_Bound::Resident_emerge () {
        Link<Camera_Bound>::link(camera_bounds);
    }
    void Camera_Bound::Resident_reclude () {
        Link<Camera_Bound>::unlink();
    }
    Vec Camera_Bound::Resident_get_pos () { return corner.c; }
    void Camera_Bound::Resident_set_pos (Vec p) { corner.c = p; }

} using namespace geo;

HACCABLE(Default_Camera) {
    name("geo::Default_Camera");
    attr("pos", member(&Default_Camera::pos).optional());
    finish([](Default_Camera& v){ v.finish(); });
}

HACCABLE(Camera_Bound) {
    name("geo::Camera_Bound*");
    attr("corner", member(&Camera_Bound::corner));
    attr("right", member(&Camera_Bound::right));
    finish(&Camera_Bound::finish);
}
