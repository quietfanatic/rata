
#include "../inc/camera.h"
#include "../inc/rooms.h"
#include "../../core/inc/window.h"

namespace geo {
    using namespace core;

    bool camera_jump = false;
    bool free_camera = false;

    Vec camera_pos = Vec(10, 7.5);

    void set_camera_pos (Vec p) {
        if (!free_camera)
            camera_pos = p;
    }

    Vec& update_camera () {
         // TODO: do stuff
        if (free_camera && !window->cursor_trapped) {
            float move_speed = 1/20.0;
            if (window->cursor_pos.x < 5) {
                camera_pos.x += (window->cursor_pos.x - 5) * move_speed;
            }
            else if (window->cursor_pos.x > window->width*PX - 5) {
                camera_pos.x += (window->cursor_pos.x - window->width*PX + 5) * move_speed;
            }
            if (window->cursor_pos.y < 5) {
                camera_pos.y -= (window->cursor_pos.y - 5) * move_speed;
            }
            else if (window->cursor_pos.y > window->height*PX - 5) {
                camera_pos.y -= (window->cursor_pos.y - window->height*PX + 5) * move_speed;
            }
        }
        if (!camera_pos.is_defined())
            camera_pos = Vec(10, 7.5);
        return camera_pos;
    }

}
