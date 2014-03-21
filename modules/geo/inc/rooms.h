#ifndef HAVE_GEO_ROOMS_H
#define HAVE_GEO_ROOMS_H

#include <vector>
#include "util/inc/geometry.h"
#include "util/inc/organization.h"

namespace geo {
    using namespace util;

     // A resident is something that belongs to a room.
    struct Resident;

    struct Room : Link<Room> {
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

    struct Resident : Link<Resident> {
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
         // These are primarily for the editor
        virtual Vec Resident_get_pos () { return Vec(NAN, NAN); }
        virtual void Resident_set_pos (Vec p) { }
        virtual size_t Resident_n_pts () { return 0; }
         // These should be relative to get_pos
        virtual Vec Resident_get_pt (size_t i) { return Vec(NAN, NAN); }
        virtual void Resident_set_pt (size_t i, Vec) { }
        virtual Rect Resident_boundary () { return Rect(-0.25, -0.25, 0.25, 0.25); }

        virtual ~Resident () { }
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
