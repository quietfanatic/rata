#include "geo/inc/camera.h"

#include "hacc/inc/haccable_standard.h"
#include "vis/inc/color.h"
using namespace core;
using namespace vis;

namespace geo {

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
        deactivate();
        prev = camera;
        camera = this;
        active = true;
    }
    void Camera::deactivate () {
        if (active) {
            for (Camera** c = &camera; *c; c = &(*c)->prev) {
                if (*c == this) {
                    *c = prev;
                    prev = NULL;
                    return;
                }
            }
        }
        prev = NULL;
        active = false;
    }

    Default_Camera& default_camera () {
        static Default_Camera r (true);
        return r;
    }

    Links<Camera_Bound> camera_bounds;

    void Camera_Bound::set_right (Camera_Bound* o) {
        if (right) right->left = NULL;
        if (o) {
            if (o->left) o->left->set_right(NULL);
            o->left = this;
        }
        right = o;
    }
    void Camera_Bound::finish () {
        if (right) {
            edge = double_tangent(corner, right->corner);
        }
        else {
            edge = Line();
        }
    }
    void Camera_Bound::Resident_emerge () {
        printf("Camera_Bound::Resident_emerge()\n");
        Link<Camera_Bound>::link(camera_bounds);
    }
    void Camera_Bound::Resident_reclude () {
        Link<Camera_Bound>::unlink();
    }
    Vec Camera_Bound::Resident_get_pos () { return corner.c; }
    void Camera_Bound::Resident_set_pos (Vec p) {
        corner.c = p;
        finish();
        if (left) left->finish();
    }
    size_t Camera_Bound::Resident_n_pts () { return 1; }
    Vec Camera_Bound::Resident_get_pt (size_t i) {
        if (right)
            return edge.a - corner.c;
        else
            return Vec(corner.r, 0);
    }
    void Camera_Bound::Resident_set_pt (size_t i, Vec p) {
        if (right) {
             // Figure out whether the user means negative or positive radius
            float mdist = (double_tangent(Circle(corner.c, -p.mag()), right->corner).a - (corner.c + p)).mag();
            float pdist = (double_tangent(Circle(corner.c, p.mag()), right->corner).a - (corner.c + p)).mag();
            if (mdist < pdist)
                corner.r = -p.mag();
            else
                corner.r = p.mag();
        }
        else {
            corner.r = p.x;
        }
        finish();
        if (left) left->finish();
    }

    void Camera_Bound::Resident_debug_draw () {
        color_offset(Vec(0, 0));
        draw_color(0xff00ffff);
        draw_circle(corner);
        if (edge.is_defined()) {
            draw_line(edge.a, edge.b);
        }
    }
    Camera_Bound::~Camera_Bound () {
        if (right) right->left = NULL;
    }

} using namespace geo;

HACCABLE(Default_Camera) {
    name("geo::Default_Camera");
    attr("pos", member(&Default_Camera::pos).optional());
    finish([](Default_Camera& v){ v.finish(); });
}

HACCABLE(Camera_Bound) {
    name("geo::Camera_Bound");
    attr("Resident", base<Resident>().collapse());
    attr("corner", member(&Camera_Bound::corner));
    attr("right", value_methods(&Camera_Bound::get_right, &Camera_Bound::set_right));
    finish(&Camera_Bound::finish);
}
