
#include "../inc/camera.h"
#include "../inc/rooms.h"
#include "../../core/inc/phases.h"
#include "../../vis/inc/sprites.h"

namespace geo {

    bool camera_jump = false;

    struct Camera : core::Layer {
        Camera () : core::Layer("A.M", "camera", true) { }
        Vec pos = Vec(10, 7.5);

         // Layer
        void start () { }
        void run () {
            if (Resident* r = beholding()) {
                pos = r->resident_pos();
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
