#include "geo/inc/vision.h"

#include "core/inc/commands.h"
#include "geo/inc/camera.h"
#include "util/inc/debug.h"
#include "vis/inc/color.h"
using namespace vis;

namespace geo {

    Links<Wall> walls;

    void Wall::set_left (Wall* o) {
        if (left) left->right = NULL;
        if (o) {
            if (o->right) o->right->set_left(NULL);
            o->right = this;
        }
        left = o;
    }
    void Wall::finish () {
        if (left) {
            edge = double_tangent(corner, left->corner);
        }
        else {
            edge = Line();
        }
        Resident::finish();
    }
    void Wall::Resident_emerge () {
        printf("Wall::Resident_emerge()\n");
        Link<Wall>::link(walls);
    }
    void Wall::Resident_reclude () {
        Link<Wall>::unlink();
    }
    Vec Wall::Resident_get_pos () { return corner.c; }
    void Wall::Resident_set_pos (Vec p) {
        corner.c = p;
        finish();
        if (right) right->finish();
    }
    size_t Wall::Resident_n_pts () { return 2; }
    Vec Wall::Resident_get_pt (size_t i) {
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
    void Wall::Resident_set_pt (size_t i, Vec p) {
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
            Wall* best_wall = NULL;
            for (auto& wall : walls) {
                float dist2 = length2((p + corner.c) - wall.corner.c);
                if (dist2 < best_dist2) {
                    best_dist2 = dist2;
                    best_wall = &wall;
                }
            }
            if (best_wall) {
                set_left(best_wall == this ? NULL : best_wall);
                finish();
            }
        }
    }
    Wall::~Wall () {
        if (left) left->right = NULL;
        if (right) right->left = NULL;
    }

///// CAMERA CONSTRAINT SATISFACTION ALGORITHM /////
 // Basically, we have a list of rectangles in priority order.  The algorithm
 //  tries to find the largest prefix of that list where it can find a valid
 //  point both within all the rectangles in that prefix and within all the
 //  active Camera_Bounds in the map.

    static void insert_attn (Vision& v, const Vision::Attn& a, size_t i) {
        if (i >= v.MAX_ATTNS) return;
        if (i == v.n_attns || a.priority > v.attns[i].priority) {
            insert_attn(v, v.attns[i], i + 1);
            v.attns[i] = a;
            v.n_attns += 1;
        }
        else insert_attn(v, a, i + 1);
    }

    void Vision::look_at (const Rect& r, float priority) {
        Attn attn = {r, priority};
        attn.area.l -= 9;
        attn.area.b -= 6.5;
        attn.area.r += 9;
        attn.area.t += 6.5;
        insert_attn(*this, attn, 0);
    };

     // Temporary for debugging purposes
    static std::vector<Vec> dbg_snaps;
    static Vec dbg_ideal_pos;
    static Rect dbg_area;

     // This tries to find a valid camera position within the given rectangle.
     //  If it's not possible, returns Vec(NAN, NAN)
    static Vec attempt_constraint (Vec preferred, const Rect& bound, bool debug_draw_this) {
        log("vision", "Checking %f %f within %f %f %f %f.", preferred.x, preferred.y, bound.l, bound.b, bound.r, bound.t);
        if (debug_draw_this) dbg_snaps.resize(0);
        bool currently_violating = false;
        float closest_snap_dist2 = INF;
        Vec selected_snap;
         // First check if we're violating any walls and can simply snap to one
        size_t i = 0;
        for (auto& wall : walls) {
            if (!defined(wall.edge)) continue;
            if (contains(bound_a(wall.edge), preferred)) {
                 // Check the edge
                if (contains(bound_b(wall.edge), preferred)) {
                    bool violating = contains(wall.edge, preferred);
                    Vec snap_here = snap(wall.edge, preferred);
                    if (debug_draw_this) dbg_snaps.push_back(snap_here);
                    float snap_dist2 = length2(snap_here - preferred);
                    if (snap_dist2 < closest_snap_dist2) {
                        currently_violating = violating;
                        closest_snap_dist2 = snap_dist2;
                         // Record this snap if it's in the bound
                        if (violating && covers(bound, snap_here)) {
                            log("vision", "  Violating edge of wall %lu at %f %f", i, snap_here.x, snap_here.y);
                            selected_snap = snap_here;
                        }
                        else if (violating) {
                            log("vision", "  Violation of edge of wall %lu at %f %f is oob", i, snap_here.x, snap_here.y);
                        }
                        else {
                            log("vision", "  Not violating edge of wall %lu at %f %f", i, snap_here.x, snap_here.y);
                        }
                    }
                    else {
                        log("vision", "  Snap to edge of wall %lu at %f %f is too far", i, snap_here.x, snap_here.y);
                    }
                }
                else {
                    log("vision", "  Not checking edge of wall %lu", i);
                }
            }
            else {
                 // Check the corner
                if (wall.right && defined(wall.right->edge) && !contains(bound_b(wall.right->edge), preferred)) {
                    bool violating = contains(wall.corner, preferred);
                    Vec snap_here = snap(wall.corner, preferred);
                    if (debug_draw_this) dbg_snaps.push_back(snap_here);
                    float snap_dist2 = length2(snap_here - preferred);
                    if (snap_dist2 < closest_snap_dist2) {
                        currently_violating = violating;
                        closest_snap_dist2 = snap_dist2;
                        if (violating && covers(bound, snap_here)) {
                            log("vision", "  Violating corner of wall %lu at %f %f", i, snap_here.x, snap_here.y);
                            selected_snap = snap_here;
                        }
                        else if (violating) {
                            log("vision", "  Violation of corner of wall %lu at %f %f is oob", i, snap_here.x, snap_here.y);
                        }
                        else {
                            log("vision", "  Not violating corner of wall %lu at %f %f", i, snap_here.x, snap_here.y);
                        }
                    }
                    else {
                        log("vision", "  Snap to corner of wall %lu at %f %f is too far", i, snap_here.x, snap_here.y);
                    }
                }
                else {
                    log("vision", "  Not checking corner of wall %lu", i);
                }
            }
            i += 1;
        }
        if (closest_snap_dist2 == INF) {
            log("vision", "    No walls were checked!?");
        }
         // Now, did we not actually violate any walls?
        if (!currently_violating) {
            log("vision", "    Preferred position is in bounds.");
            return preferred;  // Great!
        }
         // Did we violate a wall, but its snap is in bounds?
        if (defined(selected_snap)) {
            log("vision", "    Snapping to %f %f.", selected_snap.x, selected_snap.y);
            return selected_snap;
        }
         // Ugh, now we have to enumerate all the ways the bound intersects the walls.
        closest_snap_dist2 = INF;
        i = 0;
        for (auto& wall : walls) {
            if (!defined(wall.edge)) continue;
             // Try the edge first
            Rect edge_aabb = bounds(wall.edge);
            Rect q = bound & edge_aabb;
            log("vision", "[%f %f %f %f] & [%f %f %f %f] = [%f %f %f %f]",
                bound.l, bound.b, bound.r, bound.t,
                edge_aabb.l, edge_aabb.b, edge_aabb.r, edge_aabb.t,
                q.l, q.b, q.r, q.t
            );
            if (proper(bound & edge_aabb)) {
                log("vision", "Trying intersect on edge of wall %lu\n", i);
                 // Try intersecting all four walls
                if (bound.l >= edge_aabb.l && bound.l <= edge_aabb.r) {
                    Vec snap_here = Vec(bound.l, wall.edge.y_at_x(bound.l));
                    if (debug_draw_this) dbg_snaps.push_back(snap_here);
                    float snap_dist2 = length2(snap_here - preferred);
                    if (snap_here.y >= bound.b && snap_here.y <= bound.t) {
                        if (snap_dist2 < closest_snap_dist2) {
                            closest_snap_dist2 = snap_dist2;
                            selected_snap = snap_here;
                        }
                    }
                }
                if (bound.b >= edge_aabb.b && bound.b <= edge_aabb.t) {
                    Vec snap_here = Vec(wall.edge.x_at_y(bound.b), bound.b);
                    if (debug_draw_this) dbg_snaps.push_back(snap_here);
                    float snap_dist2 = length2(snap_here - preferred);
                    if (snap_here.x >= bound.l && snap_here.x <= bound.r) {
                        if (snap_dist2 < closest_snap_dist2) {
                            closest_snap_dist2 = snap_dist2;
                            selected_snap = snap_here;
                        }
                    }
                }
                if (bound.r >= edge_aabb.l && bound.r <= edge_aabb.r) {
                    Vec snap_here = Vec(bound.r, wall.edge.y_at_x(bound.r));
                    if (debug_draw_this) dbg_snaps.push_back(snap_here);
                    float snap_dist2 = length2(snap_here - preferred);
                    if (snap_here.y >= bound.b && snap_here.y <= bound.t) {
                        if (snap_dist2 < closest_snap_dist2) {
                            closest_snap_dist2 = snap_dist2;
                            selected_snap = snap_here;
                        }
                    }
                }
                if (bound.t >= edge_aabb.b && bound.t <= edge_aabb.t) {
                    Vec snap_here = Vec(wall.edge.x_at_y(bound.t), bound.t);
                    if (debug_draw_this) dbg_snaps.push_back(snap_here);
                    float snap_dist2 = length2(snap_here - preferred);
                    if (snap_here.x >= bound.l && snap_here.x <= bound.r) {
                        if (snap_dist2 < closest_snap_dist2) {
                            closest_snap_dist2 = snap_dist2;
                            selected_snap = snap_here;
                        }
                    }
                }
            }
            else {
                log("vision", "Not trying intersect on edge of wall %lu\n", i);
            }
             // Now try the corner
            if (!wall.right || !defined(wall.right->edge)) continue;
            Rect corner_aabb = bounds(wall.corner);
            q = bound & corner_aabb;
            log("vision", "[%f %f %f %f] & [%f %f %f %f] = [%f %f %f %f]",
                bound.l, bound.b, bound.r, bound.t,
                corner_aabb.l, corner_aabb.b, corner_aabb.r, corner_aabb.t,
                q.l, q.b, q.r, q.t
            );
            if (proper(bound & corner_aabb)) {
                log("vision", "Trying intersect on corner of wall %lu\n", i);
                Line bound_l = bound_a(wall.edge);
                Line bound_r = bound_b(wall.right->edge);
                 // Try intersecting it with all four walls
                if (bound.l >= corner_aabb.l && bound.l <= corner_aabb.r) {
                     // Pythagoreas yields two intersections.
                    float y_from_corner = sqrt(
                        wall.corner.r * wall.corner.r - (bound.l-wall.corner.c.x) * (bound.l-wall.corner.c.x)
                    );
                     // Try higher intersection
                    Vec snap_here = Vec(bound.l, wall.corner.c.y + y_from_corner);
                    if (debug_draw_this) dbg_snaps.push_back(snap_here);
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
                    snap_here.y = wall.corner.c.y - y_from_corner;
                    if (debug_draw_this) dbg_snaps.push_back(snap_here);
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
                        wall.corner.r * wall.corner.r - (bound.b-wall.corner.c.y) * (bound.b-wall.corner.c.y)
                    );
                    Vec snap_here = Vec(wall.corner.c.x + x_from_corner, bound.b);
                    if (debug_draw_this) dbg_snaps.push_back(snap_here);
                    float snap_dist2 = length2(snap_here - preferred);
                    if (snap_here.x >= bound.l && snap_here.x <= bound.r
                            && !contains(bound_l, snap_here)
                            && !contains(bound_r, snap_here)) {
                        if (snap_dist2 < closest_snap_dist2) {
                            closest_snap_dist2 = snap_dist2;
                            selected_snap = snap_here;
                        }
                    }
                    snap_here.x = wall.corner.c.x - x_from_corner;
                    if (debug_draw_this) dbg_snaps.push_back(snap_here);
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
                        wall.corner.r * wall.corner.r - (bound.r-wall.corner.c.x) * (bound.r-wall.corner.c.x)
                    );
                    Vec snap_here = Vec(bound.r, wall.corner.c.y + y_from_corner);
                    if (debug_draw_this) dbg_snaps.push_back(snap_here);
                    float snap_dist2 = length2(snap_here - preferred);
                    if (snap_here.y >= bound.b && snap_here.y <= bound.t
                            && !contains(bound_l, snap_here)
                            && !contains(bound_r, snap_here)) {
                        if (snap_dist2 < closest_snap_dist2) {
                            closest_snap_dist2 = snap_dist2;
                            selected_snap = snap_here;
                        }
                    }
                    snap_here.y = wall.corner.c.y - y_from_corner;
                    if (debug_draw_this) dbg_snaps.push_back(snap_here);
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
                        wall.corner.r * wall.corner.r - (bound.t-wall.corner.c.y) * (bound.t-wall.corner.c.y)
                    );
                    Vec snap_here = Vec(wall.corner.c.x + x_from_corner, bound.t);
                    if (debug_draw_this) dbg_snaps.push_back(snap_here);
                    float snap_dist2 = length2(snap_here - preferred);
                    if (snap_here.x >= bound.l && snap_here.x <= bound.r
                            && !contains(bound_l, snap_here)
                            && !contains(bound_r, snap_here)) {
                        if (snap_dist2 < closest_snap_dist2) {
                            closest_snap_dist2 = snap_dist2;
                            selected_snap = snap_here;
                        }
                    }
                    snap_here.x = wall.corner.c.x - x_from_corner;
                    if (debug_draw_this) dbg_snaps.push_back(snap_here);
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
            else {
                log("vision", "Not trying intersect on corner of wall %lu\n", i);
            }
            i++;
        }
         // Finally return the best wall&bound intersection.
        log("vision", "Done checking %f %f within %f %f %f %f.", preferred.x, preferred.y, bound.l, bound.b, bound.r, bound.t);
        log("vision", "    Snapping to intersection at %f %f.", selected_snap.x, selected_snap.y);
        return selected_snap;
    }

     // This gets a valid camera position that's within as many attentions
     //  as possible.
    Vec Vision::get_pos (bool debug_draw_this) {
         // TODO: let the focus control position as much as possible
        if (debug_draw_this) dbg_ideal_pos = focus;
        Vec best_so_far = focus;
        Rect attn_bound = Rect(-INF, -INF, INF, INF);
        if (debug_draw_this) dbg_area = attn_bound;
        for (size_t i = 0; i < n_attns; i++) {
            attn_bound &= attns[i].area;
            Vec attempt = attempt_constraint(focus, attn_bound, debug_draw_this);
            if (!defined(attempt)) break;
            best_so_far = attempt;
            if (debug_draw_this) dbg_area = attn_bound;
        }
        n_attns = 0;
        return best_so_far;
    }

    struct Vision_Debug : vis::Drawn<vis::Overlay> {
        void Drawn_draw (vis::Overlay) override {
            color_offset(Vec(0, 0));
            draw_color(0x00ffffff);
            for (auto snap : dbg_snaps) {
                draw_circle(Circle(snap, 0.2));
            }
            draw_color(0xff00ffff);
            color_offset(Vec(0, 0));
            size_t i = 0;
            for (auto& cb : walls) {
                draw_circle(cb.corner);
                if (defined(cb.edge)) {
                    draw_line(cb.edge.a, cb.edge.b);
                }
            }
            draw_color(0xffff00ff);
            draw_circle(Circle(dbg_ideal_pos, 0.2));
            draw_rect(dbg_area);
        }
    };
    static Vision_Debug vision_debug;

} using namespace geo;

HACCABLE(Wall) {
    name("geo::Wall");
    attr("Resident", base<Resident>().collapse());
    attr("corner", member(&Wall::corner));
    attr("left", value_methods(&Wall::get_left, &Wall::set_left).optional());
    finish(&Wall::finish);
}

void _vision_debug () {
    if (vision_debug.visible())
        vision_debug.disappear();
    else
        vision_debug.appear();
}

core::New_Command _vision_debug_cmd ("vision_debug", "Toggle vision debug view", 0, _vision_debug);

