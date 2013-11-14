
#include "../inc/camera.h"
#include "../inc/rooms.h"

namespace geo {
    using namespace core;

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
        prev = camera;
        camera = this;
    }
    void Camera::deactivate () {
        for (Camera** c = &camera; *c; c = &(*c)->prev) {
            if (*c == this) {
                *c = prev;
                prev = NULL;
                return;
            }
        }
        prev = NULL;
    }

    Default_Camera& default_camera () {
        static Default_Camera r (true);
        return r;
    }

} using namespace geo;

HACCABLE(Default_Camera) {
    name("geo::Default_Camera");
    attr("pos", member(&Default_Camera::pos).optional());
    finish([](Default_Camera& v){ v.finish(); });
}
