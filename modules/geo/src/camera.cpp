
#include "../inc/camera.h"
#include "../inc/rooms.h"

namespace geo {

    bool camera_jump = false;

    Vec camera_pos = Vec(10, 7.5);

    Vec& update_camera () {
        if (Resident* r = beholding()) {
            camera_pos = r->Resident_pos();
        }
        if (!camera_pos.is_defined())
            camera_pos = Vec(10, 7.5);
        return camera_pos;
    }

}
