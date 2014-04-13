#include "geo/inc/vision.h"

#include "core/inc/commands.h"
#include "util/inc/debug.h"
#include "vis/inc/color.h"
#include "vis/inc/common.h"
using namespace vis;

namespace geo {

    Links<Wall> walls;

    float Wall::max_curve () {
        float lr = left ? length(left->pos - pos) - left->curve : INF;
        float rr = right ? length(right->pos - pos) - right->curve : INF;
        return fmin(lr, rr);
    }
    bool Wall::convex () {
        if (!left || !right) return true;
        Vec l = left->pos - pos;
        Vec r = right->pos - pos;
         // You may recognize this as the third component of a cross product.
        return l.x * r.y - l.y * r.x > 0;
    }

    void Wall::set_left (Wall* o) {
        if (left) left->right = NULL;
        if (o) {
            if (o->right) o->right->set_left(NULL);
            o->right = this;
        }
        left = o;
        finish();
        if (right) right->finish();
        if (left) left->finish();
    }
    void Wall::finish () {
         // Calculate edge
        if (left) {
            edge.a = pos + curve * normalize(left->pos - pos);
            edge.b = left->pos + left->curve * normalize(pos - left->pos);
        }
        else {
            edge = Line();
        }
         // Calculate corner
        if (left && right) {
            if (curve == 0) {
                corner = Circle(pos, 0);
            }
            else {
                 // Diagram: a right triangle mirrored about its hypotenuse
                Vec r_left_v = curve * normalize(left->pos - pos);
                Vec r_right_v = curve * normalize(right->pos - pos);
                Vec r_mid = (r_left_v + r_right_v) / 2;
                float r_ratio = curve * curve / length2(r_mid);
                corner.c = pos + r_mid * r_ratio;
                corner.r = length(pos + r_right_v - corner.c);
                 // TODO: deal with this differently
                if (!convex()) corner.r *= -1;
            }
        }
        else {
            corner = Circle();
        }
        Resident::finish();
    }
    void Wall::Resident_emerge () {
        Link<Wall>::link(walls);
    }
    void Wall::Resident_reclude () {
        Link<Wall>::unlink();
    }
    Vec Wall::Spatial_get_pos () { return pos; }
    void Wall::Spatial_set_pos (Vec p) {
        pos = p;
        finish();
        if (right) right->finish();
        if (left) left->finish();
    }
    size_t Wall::Spatial_n_pts () { return 3; }
    Vec Wall::Spatial_get_pt (size_t i) {
        switch (i) {
            case 0: return Vec(curve, 0);
            case 1: return repel_point;
            case 2: return left ? left->pos - pos : Vec(0, -1);
            default: return Vec();
        }
    }
    void Wall::Spatial_set_pt (size_t i, Vec p) {
        switch (i) {
            case 0: curve = fmin(fabs(p.x), max_curve()); break;
            case 1: repel_point = p; break;
            case 2: {
                float best_dist2 = INF;
                Wall* best_wall = NULL;
                for (auto& wall : walls) {
                    float dist2 = length2(wall.pos - pos);
                    if (dist2 < best_dist2) {
                        best_dist2 = dist2;
                        best_wall = &wall;
                    }
                }
                if (best_wall && best_wall != this)
                    set_left(best_wall);
                else set_left(NULL);
                break;
            }
            default: break;
        }
        finish();
        if (left) left->finish();
        if (right) right->finish();
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

    void Vision::attend (const Rect& r, float priority) {
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
    static std::vector<Rect> dbg_areas;

     // This tries to find a valid camera position within the given rectangle.
     //  If it's not possible, returns Vec(NAN, NAN)
    static Vec attempt_constraint (Vec preferred, const Rect& bound, bool debug_draw_this) {
        log("vision", "Checking %f %f within %f %f %f %f.", preferred.x, preferred.y, bound.l, bound.b, bound.r, bound.t);
        if (debug_draw_this) dbg_snaps.resize(0);
        bool currently_violating = false;
        float closest_snap_dist2 = INF;
        Vec selected_snap;
         // First check if we're violating any walls and can simply snap to one
        for (auto& wall : walls) {
            check_edge: {
                if (!wall.left) goto check_corner;
                 // Get snap position; we'll check if it's valid later
                Vec snap_here = snap(wall.edge, preferred);
                float snap_dist2 = length2(snap_here - preferred);
                 // Account for repel points if necessary
                if (wall.repel_point != Vec(0, 0)) {
                    Vec repel_snap = intersect(wall.edge, Line(preferred, wall.pos + wall.repel_point));
                    if (length2(repel_snap - wall.left->pos) > length2(snap_here - wall.left->pos))
                        snap_here = repel_snap;
                }
                if (wall.left->repel_point != Vec(0, 0)) {
                    Vec repel_snap = intersect(wall.edge, Line(preferred, wall.left->pos + wall.left->repel_point));
                    if (length2(repel_snap - wall.pos) > length2(snap_here - wall.pos))
                        snap_here = repel_snap;
                }
                 // Didn't actually snap to the edge
                if (!covers(bounds(wall.edge), snap_here)) {
                    log("vision", "  Out of range of edge of wall %p", &wall);
                    goto check_corner;
                }
                if (debug_draw_this) dbg_snaps.push_back(snap_here);
                bool violating = contains(wall.edge, preferred);
                 // Prefer violations over non-violations; otherwise we may
                 //  miss some violations when the camera is constrained to a
                 //  one-dimensional line.
                if (snap_dist2 + !violating < closest_snap_dist2 + !currently_violating) {
                    currently_violating = violating;
                    closest_snap_dist2 = snap_dist2;
                    if (violating) {
                        log("vision", "  Violating edge of wall %p at %f %f", &wall, snap_here.x, snap_here.y);
                        selected_snap = snap_here;
                    }
                    else {
                        log("vision", "  Not violating edge of wall %p at %f %f", &wall, snap_here.x, snap_here.y);
                    }
                }
                else {
                    log("vision", "  Snap to edge of wall %p at %f %f is too far", &wall, snap_here.x, snap_here.y);
                }
            }
            check_corner: {
                if (!wall.right) goto next_wall;
                Vec snap_here = snap(wall.corner, preferred);
                bool violating;
                float snap_dist2 = length2(snap_here - preferred);
                if (wall.repel_point != Vec(0, 0)) {
                    Line itx = intersect(wall.corner, Line(preferred, wall.pos + wall.repel_point));
                    if (length2(itx.a - wall.pos) <= wall.curve*wall.curve)
                        snap_here = itx.a;
                    else if (length2(itx.b - wall.pos) <= wall.curve*wall.curve)
                        snap_here = itx.b;
                    else {
                        log("vision", "  Out of range of corner of wall %p", &wall);
                        goto next_wall;
                    }
                    violating = length2(snap_here - (wall.pos + wall.repel_point))
                              > length2(preferred - (wall.pos + wall.repel_point));
                }
                else {
                    if (length2(snap_here - wall.pos) > wall.curve*wall.curve) {
                        log("vision", "  Out of range of corner of wall %p", &wall);
                        goto next_wall;
                    }
                    violating = contains(wall.corner, preferred);
                }
                if (debug_draw_this) dbg_snaps.push_back(snap_here);
                if (snap_dist2 + !violating < closest_snap_dist2 + !currently_violating) {
                    currently_violating = violating;
                    closest_snap_dist2 = snap_dist2;
                    if (violating) {
                        log("vision", "  Violating corner of wall %p at %f %f", &wall, snap_here.x, snap_here.y);
                        selected_snap = snap_here;
                    }
                    else {
                        log("vision", "  Not violating corner of wall %p at %f %f", &wall, snap_here.x, snap_here.y);
                    }
                }
                else {
                    log("vision", "  Snap to corner of wall %p at %f %f is too far", &wall, snap_here.x, snap_here.y);
                }
            }
            next_wall: { }
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
        if (defined(selected_snap) && covers(bound, selected_snap)) {
            log("vision", "    Snapping to %f %f.", selected_snap.x, selected_snap.y);
            return selected_snap;
        }
         // Ugh, now we have to enumerate all the ways the bound intersects the walls.
        closest_snap_dist2 = INF;
        preferred = selected_snap;
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
                log("vision", "Trying intersect on edge of wall %p", &wall);
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
                log("vision", "Not trying intersect on edge of wall %p", &wall);
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
                log("vision", "Trying intersect on corner of wall %p", &wall);
                Line bound_l = bound_a(wall.edge);
                Line bound_r = bound_b(wall.right->edge);
                 // Try intersecting it with all four walls
                if (bound.l >= corner_aabb.l && bound.l <= corner_aabb.r) {
                     // Pythagoreas yields two intersections.
                    float y_from_corner = sqrt(
                          wall.corner.r * wall.corner.r
                        - (bound.l-wall.corner.c.x) * (bound.l-wall.corner.c.x)
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
                          wall.corner.r * wall.corner.r
                        - (bound.b-wall.corner.c.y) * (bound.b-wall.corner.c.y)
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
                          wall.corner.r * wall.corner.r
                        - (bound.r-wall.corner.c.x) * (bound.r-wall.corner.c.x)
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
                          wall.corner.r * wall.corner.r
                        - (bound.t-wall.corner.c.y) * (bound.t-wall.corner.c.y)
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
                log("vision", "Not trying intersect on corner of wall %p", &wall);
            }
        }
         // Finally return the best wall&bound intersection.
        log("vision", "Done checking %f %f within %f %f %f %f.", preferred.x, preferred.y, bound.l, bound.b, bound.r, bound.t);
        log("vision", "    Snapping to intersection at %f %f.", selected_snap.x, selected_snap.y);
        return selected_snap;
    }

     // This gets a valid camera position that's within as many attentions
     //  as possible.
    Vec Vision::look (Vec origin, Vec* focus, bool debug_draw_this) {
        Vec rel = *focus - origin;
         // Preliminary focus restriction.  TODO: leave this to the caller
        if (rel.x < -18) {
            rel = Vec(-18, -18 * slope(rel));
        }
        else if (rel.x > 18) {
            rel = Vec(18, 18 * slope(rel));
        }
        if (rel.y < -13) {
            rel = Vec(-13 / slope(rel), -13);
        }
        else if (rel.y > 13) {
            rel = Vec(13 / slope(rel), 13);
        }
         // Secondary focus restriction (restrict to walls)
        Vec middle = (rel + origin + origin) / 2;
        middle = attempt_constraint(middle, origin + Rect(-9, -6.5, 9, 6.5), false);
        if (!defined(middle)) return (*focus + origin) / 2;  // Something went wrong
        Vec middle_rel = middle - origin;
         // Tertiary focus restriction
        if (rel.x < middle_rel.x - 10) {
            rel = Vec(middle_rel.x - 10, (middle_rel.x - 10) * slope(rel));
        }
        else if (rel.x > middle_rel.x + 10) {
            rel = Vec(middle_rel.x + 10, (middle_rel.x + 10) * slope(rel));
        }
        if (rel.y < middle_rel.y - 7.5) {
            rel = Vec((middle_rel.y - 7.5) / slope(rel), middle_rel.y - 7.5);
        }
        else if (rel.y > middle_rel.y + 7.5) {
            rel = Vec((middle_rel.y + 7.5) / slope(rel), middle_rel.y + 7.5);
        }
        *focus = rel + origin;
         // Now allow the area around the cursor to stretch by one block
        Rect cursor_range = *focus + Rect(-9, -6.5, 9, 6.5);
        if (cursor_range.r < middle.x) {
            cursor_range.r = middle.x;
        }
        else if (cursor_range.l > middle.x) {
            cursor_range.l = middle.x;
        }
        if (cursor_range.t < middle.y) {
            cursor_range.t = middle.y;
        }
        else if (cursor_range.b > middle.y) {
            cursor_range.b = middle.y;
        }
         // Keep two separate bounds so we can account for cursor motion
         //  within the non-cursor range.
        Rect without_cursor = Rect(-INF, -INF, INF, INF);
        Rect with_cursor = cursor_range;
        Vec best_so_far = (*focus + origin) / 2;
        if (debug_draw_this) {
            dbg_areas.resize(0);
            dbg_areas.push_back(cursor_range);
            for (size_t i = 0; i < n_attns; i++) {
                dbg_areas.push_back(attns[i].area);
            }
        }
         // Now iterate over attentions to find the most we can satisfy
        for (size_t i = 0; i < n_attns; i++) {
            with_cursor &= attns[i].area;
            without_cursor &= attns[i].area;
            if (!defined(with_cursor)) break;
             // The position of ideal in without_cursor is proportional to
             // the position of the cursor in with_cursor...or something like that?
            Vec ideal = (cursor_range.rt() - without_cursor.lb())
                      / (cursor_range.size() + without_cursor.size())
                      * without_cursor.size()
                      + without_cursor.lb();
            ideal = constrain(with_cursor, ideal);
            Vec attempt = attempt_constraint(ideal, with_cursor, debug_draw_this);
            if (!defined(attempt)) break;
            dbg_ideal_pos = ideal;
            best_so_far = attempt;
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
                if (cb.left && cb.right) {
                    if (cb.convex()) {
                        draw_arc(cb.corner,
                            angle(cb.right->edge.b - cb.corner.c),
                            angle(cb.edge.a - cb.corner.c)
                        );
                    }
                    else {
                        draw_arc(cb.corner,
                            angle(cb.corner.c - cb.edge.a),
                            angle(cb.corner.c - cb.right->edge.b)
                        );
                    }
                }
                draw_line(cb.pos, cb.pos + cb.repel_point);
                if (defined(cb.edge)) {
                    draw_line(cb.edge.a, cb.edge.b);
                }
            }
            draw_color(0xffff00ff);
            draw_circle(Circle(dbg_ideal_pos, 0.2));
            for (auto& a : dbg_areas) {
                draw_rect(a);
            }
        }
    };
    static Vision_Debug vision_debug;

} using namespace geo;

HACCABLE(Wall) {
    name("geo::Wall");
    attr("Resident", base<Resident>().collapse());
    attr("pos", member(&Wall::pos));
    attr("curve", member(&Wall::curve).optional());
    attr("repel_point", member(&Wall::repel_point).optional());
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

