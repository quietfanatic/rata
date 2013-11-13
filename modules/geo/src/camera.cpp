
#include "../inc/camera.h"
#include "../inc/rooms.h"

namespace geo {

    bool camera_jump = false;
    bool free_camera = false;

    Vec camera_pos = Vec(10, 7.5);

    void set_camera_pos (Vec p) {
        if (!free_camera)
            camera_pos = p;
    }

    Vec& update_camera () {
         // TODO: do stuff
        if (free_camera) {
        }
        if (!camera_pos.is_defined())
            camera_pos = Vec(10, 7.5);
        return camera_pos;
    }

}
