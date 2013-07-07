
#include "../inc/rooms.h"
#include "../../core/inc/phases.h"
#include "../../util/inc/debug.h"
#include "../../hacc/inc/everything.h"

namespace geo {

    Logger geo_logger ("geo");
    Room* current_room = NULL;
    Resident* beholder = NULL;
    Room tumbolia;

    void Room::activate () {
        geo_logger.log("Activating room @%lx", (unsigned long)this);
        for (Resident* r = residents.first(); r; r = r->next())
            r->emerge();
    }
    void Room::deactivate () {
        geo_logger.log("Deactivating room @%lx", (unsigned long)this);
        for (Resident* r = residents.first(); r; r = r->next())
            r->reclude();
    }

    void enter (Room* r) {
        geo_logger.log("Entering room @%lx", (unsigned long)r);
        if (!r) {
            geo_logger.log("Oops, tried to enter the NULL pointer.\n");
            r = &tumbolia;
        }
         // Mark activating
        r->activating = true;
        for (auto n : r->neighbors) {
            n->activating = true;
        }
         // Deactivate
        if (current_room) {
            for (auto crn : current_room->neighbors) {
                if (!crn->activating) {
                    crn->active = false;
                    crn->deactivate();
                }
            }
            if (!current_room->activating) {
                current_room->active = false;
                current_room->deactivate();
            }
        }
         // Activate
        current_room = r;
        if (!r->active)
            r->activate();
        r->activating = false;
        for (auto n : r->neighbors) {
            if (!n->active)
                n->activate();
            n->activating = false;
        }
    }

    void behold (Resident* res) {
        beholder = res;
        enter(res->room);
    }

    Room::~Room () {
        if (current_room == this) current_room = NULL;
    }

    void Resident::finish () { if (room) link(room->residents); }

    void Resident::reroom (Vec pos) {
        if (!room) room = current_room;
        Room* origin =
            room == &tumbolia
                ? current_room : room;
        if (!origin->boundary.covers(pos)) {
            for (auto n : origin->neighbors) {
                if (n->boundary.covers(pos)) {
                    room = n;
                    link(n->residents);
                    if (beholder == this)
                        enter(n);
                    else if (!n->active)
                        reclude();
                    return;
                }
            }
            if (room != &tumbolia) {
                if (beholder != this)
                    geo_logger.log("Resident @%lx ended up in tumbolia.", this);
                else
                    geo_logger.log("The Beholder has left the building.  Party's over.", this);
                room = &tumbolia;
                link(tumbolia.residents);
                reclude();
            }
        }
    }

} using namespace geo;

HCB_BEGIN(Room)
    name("geo::Room");
    attr("boundary", member(&Room::boundary));
    attr("neighbors", member(&Room::neighbors));
HCB_END(Room)


HCB_BEGIN(Resident)
    name("geo::Resident");
    attr("room", member(&Resident::room));
HCB_END(Resident)
