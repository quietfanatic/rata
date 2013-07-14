
#include "../inc/camera.h"
#include "../inc/rooms.h"
#include "../../core/inc/phases.h"
#include "../../vis/inc/common.h"

namespace geo {

    bool camera_jump = false;

    struct Camera : core::Phase {
        Camera () : core::Phase("Z.M", "camera", true) { }
        Vec pos = Vec(10, 7.5);

        void Phase_start () override { }
        void Phase_run () override {
            if (Resident* r = beholding()) {
                pos = r->Resident_pos();
            }
            if (!pos.is_defined())
                pos = Vec(10, 7.5);
            vis::camera_pos = pos;
        }
    } camera;

    Vec get_camera_pos () {
        return camera.pos;
    }

}
