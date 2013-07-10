
#include "../inc/camera.h"
#include "../inc/rooms.h"
#include "../../core/inc/phases.h"
#include "../../vis/inc/common.h"

namespace geo {

    bool camera_jump = false;

    struct Camera : core::Layer {
        Camera () : core::Layer("A.M", "camera", true) { }
        Vec pos = Vec(10, 7.5);

        void Layer_start () override { }
        void Layer_run () override {
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
