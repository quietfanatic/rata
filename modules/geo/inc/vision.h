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
        Vec pos;
        float real_curve = 0;  // Used for boundary of vision
        float push_curve = NAN;  // Used for direction to correct vision
        Wall* left = NULL;  // Control this
         // The following are automatically set
        Wall* right = NULL;  // Automatically set
        Line edge;  // Between this and left
        Circle real_circle;
        Circle push_circle;

        bool convex ();

         // No two adjacent walls can have summed curve larger than their distance
        float max_curve ();

        void set_left (Wall*);
        Wall* get_left () const { return left; }
        void finish ();
        ~Wall();

        void Resident_emerge () override;
        void Resident_reclude () override;
        Vec Spatial_get_pos () override;
        void Spatial_set_pos (Vec) override;
        size_t Spatial_n_pts () override;
        Vec Spatial_get_pt (size_t) override;
        void Spatial_set_pt (size_t, Vec) override;
    };
    extern Links<Wall> walls;

}

#endif
