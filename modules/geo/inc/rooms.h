#ifndef HAVE_GEO_ROOMS_H
#define HAVE_GEO_ROOMS_H

#include "../../util/inc/geometry.h"
#include "../../util/inc/organization.h"

namespace geo {
    using namespace util;

     // A resident is something that belongs to a room.
    struct Resident;

    struct Room : Link<Room> {
        Rect boundary = Rect(0, 0, 20, 15);
        std::vector<Room*> neighbors;

        Links<Resident> residents;
         // The room is loaded if this is non-zero.
        uint observer_count = 0;
        void observe ();
        void forget ();
         // Rooms are not to be destructed if they have non-doomed Residents
        Room ();
        ~Room ();
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
        virtual Vec Resident_get_pos () { return Vec(NAN, NAN); }
        virtual void Resident_set_pos (Vec p) { }
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
