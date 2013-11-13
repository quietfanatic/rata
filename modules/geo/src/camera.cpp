
#include "../inc/camera.h"
#include "../inc/rooms.h"
#include "../../core/inc/window.h"

namespace geo {
    using namespace core;

    void Camera::set_pos (Vec p) {
        if (!free)
            pos = p;
    }

    Vec& Camera::update () {
         // TODO: do stuff
        if (free && !window->cursor_trapped) {
            float move_speed = 1/20.0;
            if (window->cursor_pos.x < 5) {
                pos.x += (window->cursor_pos.x - 5) * move_speed;
            }
            else if (window->cursor_pos.x > window->width*PX - 5) {
                pos.x += (window->cursor_pos.x - window->width*PX + 5) * move_speed;
            }
            if (window->cursor_pos.y < 5) {
                pos.y -= (window->cursor_pos.y - 5) * move_speed;
            }
            else if (window->cursor_pos.y > window->height*PX - 5) {
                pos.y -= (window->cursor_pos.y - window->height*PX + 5) * move_speed;
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
