#ifndef HAVE_GEO_ROOMS_H
#define HAVE_GEO_ROOMS_H

#include "../../util/inc/geometry.h"
#include "../../util/inc/organization.h"

namespace geo {
    using namespace util;

     // A resident is something that belongs to a room.
    struct Resident;

    struct Room {
        Rect boundary = Rect(-INF, -INF, INF, INF);
        bool active = false;
        bool activating = false;
        std::vector<Room*> neighbors;
        Links<Resident> residents;

        void activate ();
        void deactivate ();
        ~Room ();
    };

    struct Resident : Link<Resident> {
        Room* room = NULL;
         // Called when the room is loaded
        virtual void Resident_emerge () = 0;
         // Called when the room is unloaded
        virtual void Resident_reclude () = 0;
         // Communicates to Resident where the actor actually is.
        virtual Vec Resident_pos ();  // Default: center of room
        virtual ~Resident () { }
        void finish ();
         // Checks if the Resident's position, as reported by Resident_pos,
         //  has left its current room, and changes rooms if so.  Call this if
         //  the position changes at all.
        void reroom ();
         // Remove the Resident from all rooms.
        void deroom ();
    };

    extern Room* current_room;

    void enter (Room*);

     // The camera and the geography focus on the target of the last-created
     //  beholder.  When it's destroyed, the previous beholder takes over.
     //  If there are no beholders or the current beholder has no target,
     //  the camera and geography remain stationary.  If no beholders are ever
     //  created, no rooms will be activated.
    struct Beholder : Link<Beholder> {
        Resident* target = NULL;
        void activate ();
        void deactivate ();
    };
    extern Links<Beholder> beholders;

    Resident* beholding ();

}

#endif
