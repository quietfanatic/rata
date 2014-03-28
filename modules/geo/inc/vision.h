#ifndef HAVE_GEO_VISION_H
#define HAVE_GEO_VISION_H

#include "geo/inc/rooms.h"
#include "util/inc/geometry.h"
#include "util/inc/organization.h"

namespace geo {

     // This represents a field of vision.  This is a non-stateful struct; its
     //  fields must be repopulated every frame.
    struct Vision {
         // All fields of vision are 20x15 blocks large.  This is hard-coded
         //  because the configuration of Walls in the world rely on it.
        static CE Vec range = Vec(20, 15);
         // Register one attention for the next frame.
        void attend (const Rect& area, float priority);
         // Run the constraint satisfaction algorithm.
         //   origin: where you're looking from
         //   focus: where you're looking to (e.g. a cursor)
         //   returns a point somewhere between origin and focus accounting for
         //    all the currently active Walls.
         //   focus will be constrained to within the view of the returned point.
        Vec look (Vec origin, Vec* focus, bool debug_draw_this = false);

         // A list of priority-order areas this will try to look at.
        struct Attn {
            Rect area;
            float priority;
        };
        static CE size_t MAX_ATTNS = 4;
        Attn attns [MAX_ATTNS];
        uint32 n_attns = 0;
    };

     // These link together to form the boundaries for vision fields.
     // These constrain the CENTER of an area the size of vision_size.
    struct Wall : Link<Wall>, geo::Resident {
        Circle corner;  // Negative radius means concave corner.  Radius can be 0
        Line edge;  // Automatically set.  Between this and right.
        Wall* left = NULL;  // Control this
        Wall* right = NULL;  // Automatically set
        void set_left (Wall*);
        Wall* get_left () const { return left; }
        void finish ();
        void Resident_emerge () override;
        void Resident_reclude () override;
        Vec Resident_get_pos () override;
        void Resident_set_pos (Vec) override;
        size_t Resident_n_pts () override;
        Vec Resident_get_pt (size_t) override;
        void Resident_set_pt (size_t, Vec) override;
        ~Wall();
    };
    extern Links<Wall> walls;

}

#endif
