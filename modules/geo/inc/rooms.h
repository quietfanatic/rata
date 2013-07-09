#ifndef HAVE_GEO_ROOMS_H
#define HAVE_GEO_ROOMS_H

#include "../../util/inc/Vec.h"
#include "../../util/inc/Rect.h"
#include "../../util/inc/organization.h"

namespace geo {

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

    struct Resident : Linkable<Resident> {
        Room* room = NULL;
        virtual void Resident_emerge () = 0;
        virtual void Resident_reclude () = 0;
        virtual Vec Resident_pos ();  // Default: center of room
        virtual ~Resident () { }
        void finish ();
        void reroom ();
    };

    extern Room* current_room;
    extern Room tumbolia;

    void enter (Room*);

     // The camera and the geography focus on the target of the last-created
     //  beholder.  When it's destroyed, the previous beholder takes over.
     //  If there are no beholders or the current beholder has no target,
     //  the camera and geography remain stationary.  If no beholders are ever
     //  created, no rooms will be activated.
    struct Beholder : Linkable<Beholder> {
        Resident* target = NULL;
        void activate ();
        void deactivate ();
    };
    extern Links<Beholder> beholders;

    Resident* beholding ();

}

#endif
