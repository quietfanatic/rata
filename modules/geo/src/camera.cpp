
#include "../inc/camera.h"
#include "../inc/rooms.h"

namespace geo {
    using namespace core;

    void Camera::set_pos (Vec p) {
        if (!free)
            pos = p;
    }

    Vec& Camera::update () {
         // TODO: do stuff
        if (free && !window->cursor_trapped) {
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
        return pos;
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
                return;
            }
        }
    }
    Camera::~Camera () { deactivate(); }

} using namespace geo;

HACCABLE(Camera) {
    name("geo::Camera");
    attr("pos", member(&Camera::pos).optional());
    attr("size", member(&Camera::size).optional());
    attr("free", member(&Camera::free).optional());
    finish([](Camera& v){ v.finish(); });
}
