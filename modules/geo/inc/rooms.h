#ifndef HAVE_GEO_ROOMS_H
#define HAVE_GEO_ROOMS_H

#include "../../util/inc/Vec.h"
#include "../../util/inc/Rect.h"
#include "../../util/inc/organization.h"

namespace geo {

     // Furniture:
     //     Cannot move from its original room
     //     Is not Stateful
     // Residents:
     //     Can move between rooms
     //     Are Stateful

    struct Furniture;
    struct Resident;

    struct Room {
        Rect boundary;
        bool active = false;
        bool activating = false;
        std::vector<Room*> neighbors;
        std::vector<Furniture*> furniture;
        Links<Resident> residents;

        void activate ();
        void deactivate ();
        void enter ();
    };

    struct Furniture {
        virtual void emerge () = 0;
        virtual void reclude () = 0;
        virtual ~Furniture () { }
    };

    struct Resident : Linkable<Resident> {
        Room* room = NULL;
        virtual void emerge () = 0;
        virtual void reclude () = 0;
        virtual ~Resident () { }
        Resident ();
        void reroom (Vec pos);
    };

    extern Links<Resident> housing_office;
    extern Room* current_room;
    extern Resident* beholder;

}

#endif
