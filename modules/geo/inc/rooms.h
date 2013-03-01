#ifndef HAVE_GEO_ROOMS_H
#define HAVE_GEO_ROOMS_H

#include "../../util/inc/Vec.h"
#include "../../util/inc/Rect.h"
#include "../../util/inc/organization.h"
#include "../../core/inc/state.h"

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
        ~Room ();
    };

    struct Furniture {
        virtual void emerge () = 0;
        virtual void reclude () = 0;
        virtual void start () { }
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

    struct Geography : core::Game_Object {
        Room* current_room;
        Resident* beholder;
        Room* tumbolia;

        void enter (Room*);
        void behold (Resident*);

        Geography ();
        void start ();
        ~Geography ();
    };
    extern core::Celebrity<Geography> geography;

}

#endif
