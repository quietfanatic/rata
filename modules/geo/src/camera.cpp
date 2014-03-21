#include "geo/inc/camera.h"

#include <GL/glew.h>  // Temporary for debug drawing
#include "core/inc/commands.h"
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
    size_t Camera_Bound::Resident_n_pts () { return 2; }
    Vec Camera_Bound::Resident_get_pt (size_t i) {
        if (i == 0) {
            if (defined(edge))
                return edge.a - corner.c;
            else
                return Vec(corner.r, 0);
        }
        else if (i == 1) {
            if (left && defined(edge))
                return edge.b - corner.c;
            else
                return Vec(corner.r + 2, 0);
        }
        else return Vec();
    }
    void Camera_Bound::Resident_set_pt (size_t i, Vec p) {
        if (i == 0) {
            if (left) {
                 // Figure out whether the user means negative or positive radius
                Line medge = double_tangent(Circle(corner.c, -length(p)), left->corner);
                Line pedge = double_tangent(Circle(corner.c, length(p)), left->corner);
                float mdist = length2(medge.a - (corner.c + p));
                float pdist = length2(pedge.a - (corner.c + p));
                if (mdist < pdist) {
                    corner.r = -length(p);
                    edge = medge;
                }
                else {
                    corner.r = length(p);
                    edge = pedge;
                }
            }
            else {
                corner.r = p.x;
            }
            if (right) right->finish();
        }
        else if (i == 1) {
            float best_dist2 = INF;
            Camera_Bound* best_cb = NULL;
            for (auto& cb : camera_bounds) {
                float dist2 = length2((p + corner.c) - cb.corner.c);
                if (dist2 < best_dist2) {
                    best_dist2 = dist2;
                    best_cb = &cb;
                }
            }
            if (best_cb) {
                set_left(best_cb == this ? NULL : best_cb);
                finish();
            }
        }
    }

    Camera_Bound::~Camera_Bound () {
        if (left) left->right = NULL;
        if (right) right->left = NULL;
    }

///// CAMERA CONSTRAINT SATISFACTION ALGORITHM /////
 // This is big enough that it deserves its own section,
 //  but the API is too simple to deserve its own file.
 // Basically, we have a list of rectangles in priority order.  The algorithm
 //  tries to find the largest prefix of that list where it can find a valid
 //  point both within all the rectangles in that prefix and within all the
 //  active Camera_Bounds in the map.

    struct Attention {
        Rect area;
        double priority;
    };

    CE size_t max_attentions = 8;
    size_t current_attentions = 0;
    Attention attentions [max_attentions];

    void insert_attention (const Attention& a, size_t i) {
        if (i >= max_attentions) return;
        if (i == current_attentions || a.priority > attentions[i].priority) {
            insert_attention(attentions[i], i + 1);
            attentions[i] = a;
            current_attentions += 1;
        }
        else insert_attention(a, i + 1);
    }

    void attention (const Rect& r, double priority) {
        insert_attention(Attention{r, priority}, 0);
    };

     // Temporary for debugging purposes
    static std::vector<Vec> dbg_snaps;

     // This tries to find a valid camera position within the given rectangle.
     //  If it's not possible, returns Vec(NAN, NAN)
    Vec attempt_constraint (Vec preferred, const Rect& bound) {
        dbg_snaps.resize(0);
        bool currently_violating = true;
        float closest_snap_dist2 = INF;
        Vec selected_snap;
        for (auto& cb : camera_bounds) {
             // Check if we're violating any walls and can simply snap to one
            if (contains(bound_a(cb.edge), preferred)) {
                 // Check the edge
                if (contains(bound_b(cb.edge), preferred)) {
                    bool violating = contains(cb.edge, preferred);
                    Vec snap_here = snap(cb.edge, preferred);
                    dbg_snaps.push_back(snap_here);
                    float snap_dist2 = length2(snap_here - preferred);
                     // Consider violations to be a little closer than non-violations
                    if (snap_dist2 + !violating
                            < closest_snap_dist2 + !currently_violating) {
                        currently_violating = violating;
                        closest_snap_dist2 = snap_dist2;
                         // Record this snap if it's in the bound
                        if (violating && contains(bound, snap_here))
                            selected_snap = snap_here;
                    }
                }
            }
            else {
                 // Check the corner
                if (cb.right && defined(cb.right->edge) && contains(bound_b(cb.right->edge), preferred)) {
                    bool violating = contains(cb.corner, preferred);
                    Vec snap_here = snap(cb.corner, preferred);
                    dbg_snaps.push_back(snap_here);
                    float snap_dist2 = length2(snap_here - preferred);
                    if (snap_dist2 + !violating
                            < closest_snap_dist2 + !currently_violating) {
                        currently_violating = violating;
                        closest_snap_dist2 = snap_dist2;
                        if (violating && contains(bound, snap_here))
                            selected_snap = snap_here;
                    }
                }
            }
        }
         // Now, did we not actually violate any walls?
        if (!currently_violating && contains(bound, preferred)) {
            return preferred;  // Great!
        }
         // Did we violate a wall, but its snap is in bounds?
        if (defined(selected_snap)) {
            return selected_snap;
        }
         // Ugh, now we have to enumerate all the ways the bound intersects the walls.
        for (auto& cb : camera_bounds) {
            if (!defined(cb.edge)) continue;
             // Try the edge first
            Rect edge_aabb = bounds(cb.edge);
            if (proper(bound & edge_aabb)) {
                 // Try intersecting all four walls
                if (bound.l >= edge_aabb.l && bound.l <= edge_aabb.r) {
                    Vec snap_here = Vec(bound.l, cb.edge.y_at_x(bound.l));
                    dbg_snaps.push_back(snap_here);
                    float snap_dist2 = length2(snap_here - preferred);
                    if (snap_here.y >= bound.b && snap_here.y <= bound.t) {
                        if (snap_dist2 < closest_snap_dist2) {
                            closest_snap_dist2 = snap_dist2;
                            selected_snap = snap_here;
                        }
                    }
                }
                if (bound.b >= edge_aabb.b && bound.b <= edge_aabb.t) {
                    Vec snap_here = Vec(cb.edge.x_at_y(bound.b), bound.b);
                    dbg_snaps.push_back(snap_here);
                    float snap_dist2 = length2(snap_here - preferred);
                    if (snap_here.x >= bound.l && snap_here.x <= bound.r) {
                        if (snap_dist2 < closest_snap_dist2) {
                            closest_snap_dist2 = snap_dist2;
                            selected_snap = snap_here;
                        }
                    }
                }
                if (bound.r >= edge_aabb.l && bound.r <= edge_aabb.r) {
                    Vec snap_here = Vec(bound.r, cb.edge.y_at_x(bound.r));
                    dbg_snaps.push_back(snap_here);
                    float snap_dist2 = length2(snap_here - preferred);
                    if (snap_here.y >= bound.b && snap_here.y <= bound.t) {
                        if (snap_dist2 < closest_snap_dist2) {
                            closest_snap_dist2 = snap_dist2;
                            selected_snap = snap_here;
                        }
                    }
                }
                if (bound.t >= edge_aabb.b && bound.t <= edge_aabb.t) {
                    Vec snap_here = Vec(cb.edge.x_at_y(bound.t), bound.t);
                    dbg_snaps.push_back(snap_here);
                    float snap_dist2 = length2(snap_here - preferred);
                    if (snap_here.x >= bound.l && snap_here.x <= bound.r) {
                        if (snap_dist2 < closest_snap_dist2) {
                            closest_snap_dist2 = snap_dist2;
                            selected_snap = snap_here;
                        }
                    }
                }
            }
             // Now try the corner
            if (!cb.right || !defined(cb.right->edge)) continue;
            Rect corner_aabb = bounds(cb.corner);
            if (proper(bound & corner_aabb)) {
                Line bound_l = bound_a(cb.edge);
                Line bound_r = bound_b(cb.right->edge);
                 // Try intersecting it with all four walls
                if (bound.l >= corner_aabb.l && bound.l <= corner_aabb.r) {
                     // Pythagoreas yields two intersections.
                    float y_from_corner = sqrt(
                        cb.corner.r * cb.corner.r - (bound.l-cb.corner.c.x) * (bound.l-cb.corner.c.x)
                    );
                     // Try higher intersection
                    Vec snap_here = Vec(bound.l, cb.corner.c.y + y_from_corner);
                    dbg_snaps.push_back(snap_here);
                    float snap_dist2 = length2(snap_here - preferred);
                    if (snap_here.y >= bound.b && snap_here.y <= bound.t
                            && !contains(bound_l, snap_here)
                            && !contains(bound_r, snap_here)) {
                        if (snap_dist2 < closest_snap_dist2) {
                            closest_snap_dist2 = snap_dist2;
                            selected_snap = snap_here;
                        }
                    }
                     // Now try the lower intersection
                    snap_here.y = cb.corner.c.y - y_from_corner;
                    dbg_snaps.push_back(snap_here);
                    snap_dist2 = length2(snap_here - preferred);
                    if (snap_here.y >= bound.b && snap_here.y <= bound.t
                            && !contains(bound_l, snap_here)
                            && !contains(bound_r, snap_here)) {
                        if (snap_dist2 < closest_snap_dist2) {
                            closest_snap_dist2 = snap_dist2;
                            selected_snap = snap_here;
                        }
                    }
                }
                if (bound.b >= corner_aabb.b && bound.b <= corner_aabb.t) {
                    float x_from_corner = sqrt(
                        cb.corner.r * cb.corner.r - (bound.b-cb.corner.c.y) * (bound.b-cb.corner.c.y)
                    );
                    Vec snap_here = Vec(cb.corner.c.x + x_from_corner, bound.b);
                    dbg_snaps.push_back(snap_here);
                    float snap_dist2 = length2(snap_here - preferred);
                    if (snap_here.x >= bound.l && snap_here.x <= bound.r
                            && !contains(bound_l, snap_here)
                            && !contains(bound_r, snap_here)) {
                        if (snap_dist2 < closest_snap_dist2) {
                            closest_snap_dist2 = snap_dist2;
                            selected_snap = snap_here;
                        }
                    }
                    snap_here.x = cb.corner.c.x - x_from_corner;
                    dbg_snaps.push_back(snap_here);
                    snap_dist2 = length2(snap_here - preferred);
                    if (snap_here.x >= bound.l && snap_here.x <= bound.r
                            && !contains(bound_l, snap_here)
                            && !contains(bound_r, snap_here)) {
                        if (snap_dist2 < closest_snap_dist2) {
                            closest_snap_dist2 = snap_dist2;
                            selected_snap = snap_here;
                        }
                    }
                }
                if (bound.r >= corner_aabb.l && bound.r <= corner_aabb.r) {
                    float y_from_corner = sqrt(
                        cb.corner.r * cb.corner.r - (bound.r-cb.corner.c.x) * (bound.r-cb.corner.c.x)
                    );
                    Vec snap_here = Vec(bound.r, cb.corner.c.y + y_from_corner);
                    dbg_snaps.push_back(snap_here);
                    float snap_dist2 = length2(snap_here - preferred);
                    if (snap_here.y >= bound.b && snap_here.y <= bound.t
                            && !contains(bound_l, snap_here)
                            && !contains(bound_r, snap_here)) {
                        if (snap_dist2 < closest_snap_dist2) {
                            closest_snap_dist2 = snap_dist2;
                            selected_snap = snap_here;
                        }
                    }
                    snap_here.y = cb.corner.c.y - y_from_corner;
                    dbg_snaps.push_back(snap_here);
                    snap_dist2 = length2(snap_here - preferred);
                    if (snap_here.y >= bound.b && snap_here.y <= bound.t
                            && !contains(bound_l, snap_here)
                            && !contains(bound_r, snap_here)) {
                        if (snap_dist2 < closest_snap_dist2) {
                            closest_snap_dist2 = snap_dist2;
                            selected_snap = snap_here;
                        }
                    }
                }
                if (bound.t >= corner_aabb.b && bound.t <= corner_aabb.t) {
                    float x_from_corner = sqrt(
                        cb.corner.r * cb.corner.r - (bound.t-cb.corner.c.y) * (bound.t-cb.corner.c.y)
                    );
                    Vec snap_here = Vec(cb.corner.c.x + x_from_corner, bound.t);
                    dbg_snaps.push_back(snap_here);
                    float snap_dist2 = length2(snap_here - preferred);
                    if (snap_here.x >= bound.l && snap_here.x <= bound.r
                            && !contains(bound_l, snap_here)
                            && !contains(bound_r, snap_here)) {
                        if (snap_dist2 < closest_snap_dist2) {
                            closest_snap_dist2 = snap_dist2;
                            selected_snap = snap_here;
                        }
                    }
                    snap_here.x = cb.corner.c.x - x_from_corner;
                    dbg_snaps.push_back(snap_here);
                    snap_dist2 = length2(snap_here - preferred);
                    if (snap_here.x >= bound.l && snap_here.x <= bound.r
                            && !contains(bound_l, snap_here)
                            && !contains(bound_r, snap_here)) {
                        if (snap_dist2 < closest_snap_dist2) {
                            closest_snap_dist2 = snap_dist2;
                            selected_snap = snap_here;
                        }
                    }
                }
            }
        }
         // Finally return the best wall&bound intersection.
        return selected_snap;
    }

     // This gets a valid camera position that's within as many attentions
     //  as possible.
    Vec satisfy_most_attentions (Vec preferred) {
         // TODO: let the cursor control position as much as possible
        Vec best_so_far = preferred;
        Rect attn_bound = Rect(-INF, -INF, INF, INF);
        for (size_t i = 0; i < current_attentions; i++) {
            attn_bound &= attentions[i].area;
            Vec attempt = attempt_constraint(attn_bound.center(), attn_bound);
            if (!defined(attempt)) break;
            best_so_far = attempt;
        }
        return best_so_far;
    }

    void Default_Camera::Camera_update () {
        pos = satisfy_most_attentions(ideal_pos);
    }

    void Default_Camera::Drawn_draw (vis::Overlay) {
        color_offset(Vec(0, 0));
        draw_color(0x00ffffff);
        for (auto snap : dbg_snaps) {
            draw_circle(Circle(snap, 0.2));
        }
        draw_color(0xff00ffff);
        color_offset(Vec(0, 0));
        size_t i = 0;
        for (auto& cb : camera_bounds) {
            draw_circle(cb.corner);
            if (defined(cb.edge)) {
                draw_line(cb.edge.a, cb.edge.b);
            }
        }
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
    attr("left", value_methods(&Camera_Bound::get_left, &Camera_Bound::set_left).optional());
    finish(&Camera_Bound::finish);
}

void _camera_debug () {
    if (default_camera().visible())
        default_camera().disappear();
    else
        default_camera().appear();
}

core::New_Command _camera_debug_cmd ("camera_debug", "Toggle camera debug view", 0, _camera_debug);

