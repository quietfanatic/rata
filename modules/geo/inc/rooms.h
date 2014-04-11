#ifndef HAVE_GEO_ROOMS_H
#define HAVE_GEO_ROOMS_H

#include <vector>
#include "geo/inc/spatial.h"
#include "util/inc/geometry.h"
#include "util/inc/organization.h"

namespace geo {
    using namespace util;

     // A resident is something that belongs to a room.
    struct Resident;

    struct Room : virtual Spatial, Link<Room> {
        Rect boundary = Rect(-INF, -INF, INF, INF);
        std::vector<Room*> neighbors;
         // For making finish() idempotent
        std::vector<Room*> old_neighbors;

        Links<Resident> residents;
         // The room is loaded if this is non-zero.
        uint observer_count = 0;
        uint neighbor_observer_count = 0;
        void observe (bool neighbors = false);
        void forget (bool neighbors = false);
         // Rooms are not to be destructed if they have non-doomed Residents
        Room ();
        ~Room ();

        Vec Spatial_get_pos () override { return boundary.lb(); }
        void Spatial_set_pos (Vec pos) override { boundary = boundary + pos - boundary.lb(); }
        size_t Spatial_n_pts () override;
        Vec Spatial_get_pt (size_t) override;
        void Spatial_set_pt (size_t, Vec) override;

        void finish ();

    };
    Links<Room>& all_rooms ();

    struct Observer {
         // This room will always be loaded (if not NULL)
        Room* room = NULL;
        Room* get_room () const;
        void set_room (Room*);
         // Also load the room's neighbors?
        bool observe_neighbors = true;
        void finish ();
    };

    struct Resident : virtual Spatial, Link<Resident> {
        Room* room = NULL;

        Room* get_room () const { return room; }
        void set_room (Room*);
         // Kind of a compromise to avoid calling things on partially
         //  constructed objects.
        bool finished = false;
         // Called when the room is loaded
        virtual void Resident_emerge () = 0;
         // Called when the room is unloaded
        virtual void Resident_reclude () = 0;

        void finish ();
         // Checks if the agent has moved to a different room.  If the agent
         //  leaves the playable area, it's room is set to NULL.
        void reroom (Vec pos);
         // Like above, but if the agent leaves the playable area, it is moved
         //  back to the center of the room.
        void reroom (Vec& pos);
    };

}

#endif
