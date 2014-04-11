#ifndef HAVE_GEO_SPATIAL_H
#define HAVE_GEO_SPATIAL_H

#include "util/inc/geometry.h"

 // This role is for entities that occopy physical space.  Since spatial
 //  properties are used by a lot of classes, this should generally be
 //  virtually inherited.

namespace geo {

    struct Spatial {
        virtual util::Vec Spatial_get_pos () = 0;
        virtual void Spatial_set_pos (util::Vec) { }
        util::Vec get_pos () { return Spatial_get_pos(); }
        void set_pos (util::Vec p) { Spatial_set_pos(p); }
         // Primarily for editing purposes
        virtual size_t Spatial_n_pts () { return 0; }
        virtual util::Vec Spatial_get_pt (size_t) { return util::Vec(NAN, NAN); }
        virtual void Spatial_set_pt (size_t, util::Vec) { }
        virtual util::Rect Spatial_boundary () { return util::Rect(-0.25, -0.25, 0.25, 0.25); }
        virtual ~Spatial () { }
    };

}

#endif
