#include "geo/inc/camera.h"

#include <GL/glew.h>  // Temporary for debug drawing
#include "core/inc/commands.h"
#include "hacc/inc/haccable_standard.h"
#include "vis/inc/color.h"
using namespace core;
using namespace vis;

namespace geo {

     // Temporary for debugging purposes
    static size_t n_snaps = 0;
    static Vec snaps [128];

    void Default_Camera::Camera_update () {
        n_snaps = 0;
        bool best_snap_corner = false;
        Camera_Bound* best_snap_bound = NULL;
        float best_snap_dist2 = INF;
        Vec best_snap;
        for (auto& cb : camera_bounds) {
            if (!defined(cb.edge))
                continue;
            if (covers(bound_a(cb.edge), ideal_pos)) {
                 // Use edge
                if (covers(bound_b(cb.edge), ideal_pos)) {
                    Vec snap_v = snap(cb.edge, ideal_pos);
                    snaps[n_snaps++] = snap_v;
                    float snap_dist2 = length2(snap_v - ideal_pos);
                    if (snap_dist2 < best_snap_dist2) {
                        best_snap_bound = &cb;
                        best_snap_corner = false;
                        best_snap_dist2 = snap_dist2;
                        best_snap = snap_v;
                    }
                }
            }
            else {
                 // Use corner
                if (cb.right && defined(cb.right->edge)
                        && !covers(bound_b(cb.right->edge), ideal_pos)) {
                    Vec snap_v = snap(cb.corner, ideal_pos);
                    snaps[n_snaps++] = snap_v;
                    float snap_dist2 = length2(snap_v - ideal_pos);
                    if (snap_dist2 < best_snap_dist2) {
                        best_snap_bound = &cb;
                        best_snap_corner = true;
                        best_snap_dist2 = snap_dist2;
                        best_snap = snap_v;
                    }
                }
            }
        }
        if (best_snap_bound) {
            if (best_snap_corner
                    ? covers(best_snap_bound->corner, ideal_pos)
                    : covers(best_snap_bound->edge, ideal_pos)) {
                pos = best_snap;
                return;
            }
        }
        pos = ideal_pos;
    }

    void Default_Camera::Drawn_draw (vis::Overlay) {
        color_offset(Vec(0, 0));
        draw_color(0x00ffffff);
        for (size_t i = 0; i < n_snaps; i++) {
            draw_circle(Circle(snaps[i], 0.2));
        }
        draw_primitive(GL_POINTS, n_snaps, snaps);
        color_offset(Vec(0, 0));
        size_t i = 0;
        for (auto& cb : camera_bounds) {
            draw_color(0x0000ffff + 0x20000000 * i++);
            draw_circle(cb.corner);
            if (defined(cb.edge)) {
                draw_line(cb.edge.a, cb.edge.b + Vec(0.4, 0.4));
                draw_color(0x0000ffff);
                Line a = bound_a(cb.edge);
                draw_line(a.a, a.b + Vec(0.4, 0.4));
                Line b = bound_b(cb.edge);
                draw_line(b.a, b.b + Vec(0.4, 0.4));
            }
        }
    }

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
        if (!defined(pos))
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

    void Camera_Bound::set_left (Camera_Bound* o) {
        if (left) left->right = NULL;
        if (o) {
            if (o->right) o->right->set_left(NULL);
            o->right = this;
        }
        left = o;
    }
    void Camera_Bound::finish () {
        if (left) {
            edge = double_tangent(corner, left->corner);
        }
        else {
            edge = Line();
        }
        Resident::finish();
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
        if (right) right->finish();
    }
    size_t Camera_Bound::Resident_n_pts () { return 1; }
    Vec Camera_Bound::Resident_get_pt (size_t i) {
        if (defined(edge))
            return edge.a - corner.c;
        else
            return Vec(corner.r, 0);
    }
    void Camera_Bound::Resident_set_pt (size_t i, Vec p) {
        if (defined(edge)) {
             // Figure out whether the user means negative or positive radius
            float mdist = length2(double_tangent(Circle(corner.c, -length(p)), right->corner).a - (corner.c + p));
            float pdist = length2(double_tangent(Circle(corner.c, length(p)), right->corner).a - (corner.c + p));
            if (mdist < pdist)
                corner.r = -length(p);
            else
                corner.r = length(p);
        }
        else {
            corner.r = p.x;
        }
        finish();
        if (right) right->finish();
    }

    Camera_Bound::~Camera_Bound () {
        if (left) left->right = NULL;
        if (right) right->left = NULL;
    }

} using namespace geo;

HACCABLE(Default_Camera) {
    name("geo::Default_Camera");
    attr("pos", member(&Default_Camera::pos).optional());
    finish(&Default_Camera::finish);
}

HACCABLE(Camera_Bound) {
    name("geo::Camera_Bound");
    attr("Resident", base<Resident>().collapse());
    attr("corner", member(&Camera_Bound::corner));
    attr("left", value_methods(&Camera_Bound::get_left, &Camera_Bound::set_left));
    finish(&Camera_Bound::finish);
}

void _camera_debug () {
    if (default_camera().visible())
        default_camera().disappear();
    else
        default_camera().appear();
}

core::New_Command _camera_debug_cmd ("camera_debug", "Toggle camera debug view", 0, _camera_debug);

