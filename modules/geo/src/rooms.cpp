
#include "../inc/rooms.h"
#include "../../core/inc/phases.h"
#include "../../util/inc/debug.h"
#include "../../hacc/inc/everything.h"

namespace geo {

    Logger geo_logger ("geo");
    Room* current_room = NULL;
    Room tumbolia;
    Links<Beholder> beholders;

    void Room::activate () {
        active = true;
        geo_logger.log("Activating room @%lx", (unsigned long)this);
        size_t i = 0;
        for (auto& r : residents) {
            r.Resident_emerge();
            i++;
        }
        geo_logger.log("...and its %lu residents", i);
    }
    void Room::deactivate () {
        active = false;
        geo_logger.log("Deactivating room @%lx", (unsigned long)this);
        for (auto& r : residents)
            r.Resident_reclude();
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
                    crn->deactivate();
                }
            }
            if (!current_room->activating) {
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

    Room::~Room () {
        if (current_room == this) current_room = NULL;
    }

    void Resident::finish () {
        if (room) {
            link(room->residents);
            if (room->active)
                Resident_emerge();
        }
    }

    void Resident::reroom () {
        if (!room) room = current_room;
        Vec pos = Resident_pos();
        Room* origin =
            room == &tumbolia
                ? current_room : room;
        if (!origin->boundary.covers(pos)) {
            for (auto n : origin->neighbors) {
                if (n->boundary.covers(pos)) {
                    room = n;
                    link(n->residents);
                    if (beholding() == this)
                        enter(n);
                    else if (!n->active)
                        Resident_reclude();
                    return;
                }
            }
            if (room != &tumbolia) {
                if (beholding() != this)
                    geo_logger.log("Resident @%lx ended up in tumbolia.", this);
                else
                    geo_logger.log("The Beholder has left the building.  Party's over.", this);
                room = &tumbolia;
                link(tumbolia.residents);
                Resident_reclude();
            }
        }
    }
    void Resident::deroom () {
        Resident_reclude();
        room = NULL;
    }

    Vec Resident::Resident_pos () {
        if (room)
            return room->boundary.center();
        else
            return Vec(NAN, NAN);
    }

    void Beholder::activate () {
        link(beholders);
        if (target && target->room) {
            geo_logger.log("Behold: @%lx", (unsigned long)target);
            enter(target->room);
        }
    }
    void Beholder::deactivate () { unlink(); }

    Resident* beholding () {
        if (Beholder* b = beholders.last()) {
            return b->target;
        }
        else return NULL;
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
    finish(&Resident::finish);
HCB_END(Resident)

HCB_BEGIN(Beholder)
    name("geo::Beholder");
    attr("target", member(&Beholder::target));
    finish(&Beholder::activate);
HCB_END(Beholder)
