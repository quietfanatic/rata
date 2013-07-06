#ifndef HAVE_GEO_ROOMS_H
#define HAVE_GEO_ROOMS_H

#include "../../util/inc/Vec.h"
#include "../../util/inc/Rect.h"
#include "../../util/inc/organization.h"
#include "../../core/inc/state.h"

namespace geo {

     // A resident is something that belongs to a room.
    struct Resident;

    struct Room {
        Rect boundary;
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
        virtual void emerge () = 0;
        virtual void reclude () = 0;
        virtual ~Resident () { }
        void finish ();
        void reroom (Vec pos);
    };

    struct Geography {
        Room* current_room;
        Resident* beholder;
        Room* tumbolia;

        void enter (Room*);
        void behold (Resident*);

        Geography ();
        void finish ();
        ~Geography ();
    };
    extern Geography& geography ();

}

#endif
