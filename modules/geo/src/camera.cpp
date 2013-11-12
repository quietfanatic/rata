
#include "../inc/camera.h"
#include "../inc/rooms.h"

namespace geo {

    bool camera_jump = false;

    Vec camera_pos = Vec(10, 7.5);

    Vec& update_camera () {
         // TODO: do stuff
        if (!camera_pos.is_defined())
            camera_pos = Vec(10, 7.5);
        return camera_pos;
    }

}
