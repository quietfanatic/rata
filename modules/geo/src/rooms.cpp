
#include "../inc/rooms.h"
#include "../../core/inc/phases.h"
#include "../../util/inc/debug.h"
#include "../../hacc/inc/everything.h"

namespace geo {

    Logger geo_logger ("geo");

    static Room*& tumbolia () {
        static Room* r = hacc::reference_file<Room>("modules/geo/res/tumbolia.room");
        return r;
    }

    Links<Resident> housing_office;
    Room* current_room = NULL;
    Resident* beholder = NULL;

    void Room::activate () {
        geo_logger.log("Activating room @%lx", (unsigned long)this);
        for (auto f : furniture)
            f->emerge();
        for (Resident* r = residents.first(); r; r = r->next())
            r->emerge();
    }
    void Room::deactivate () {
        geo_logger.log("Deactivating room @%lx", (unsigned long)this);
        for (auto f : furniture)
            f->reclude();
        for (Resident* r = residents.first(); r; r = r->next())
            r->reclude();
    }

    void Room::enter () {
        geo_logger.log("Entering room @%lx", (unsigned long)this);
         // Mark activating
        activating = true;
        for (auto n : neighbors) {
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
        current_room = this;
        if (!this->active)
            this->activate();
        this->activating = false;
        for (auto n : neighbors) {
            if (!n->active)
                n->activate();
            n->activating = false;
        }
    }

    Room::~Room () {
        for (auto f : furniture) delete f;
    }

    Resident::Resident () { link(housing_office); }

    void Resident::reroom (Vec pos) {
        if (!room) room = current_room;
        Room* origin = room == tumbolia()
            ? current_room : room;
        if (!origin->boundary.covers(pos)) {
            for (auto n : origin->neighbors) {
                if (n->boundary.covers(pos)) {
                    room = n;
                    link(n->residents);
                    if (beholder == this)
                        n->enter();
                    else if (!n->active)
                        reclude();
                    return;
                }
            }
            if (room != tumbolia()) {
                if (beholder != this)
                    geo_logger.log("Resident @%lx ended up in tumbolia.", this);
                else
                    geo_logger.log("The Beholder has left the building.  Party's over.", this);
                room = tumbolia();
                link(tumbolia()->residents);
                reclude();
            }
        }
    }

    struct Room_Phase : core::Phase {
        Room_Phase () : core::Phase ("T.M") { }
        void start () {
            Resident* nextr;
            for (Resident* r = housing_office.first(); r; r = nextr) {
                nextr = r->next();
                if (r->room) {
                    r->link(r->room->residents);
                }
            }
            if (beholder && beholder->room)
                beholder->room->enter();
        }
        void stop () {
            current_room = NULL;
            beholder = NULL;
        }
    } room_phase;

}

using namespace geo;
HCB_BEGIN(Room)
    type_name("geo::Room");
    attr("boundary", member(&Room::boundary));
    attr("neighbors", member(&Room::neighbors));
    attr("furniture", member(&Room::furniture)(optional));
    finish([](Room& r){ for (auto f : r.furniture) f->start(); });
HCB_END(Room)
HCB_BEGIN(Furniture)
    type_name("geo::Furniture");
    pointee_policy(hacc::FOLLOW);
HCB_END(Furniture)
HCB_BEGIN(Resident)
    type_name("geo::Resident");
    attr("room", member(&Resident::room));
HCB_END(Resident)
