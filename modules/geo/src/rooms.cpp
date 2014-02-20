
#include "../inc/rooms.h"
#include "../../util/inc/debug.h"
#include "../../hacc/inc/everything.h"

namespace geo {

    Logger geo_logger ("geo");

    Room::Room () { link(&all_rooms()); }
    Room::~Room () { unlink(); }

    Links<Room>& all_rooms () {
        static Links<Room> r;
        return r;
    }

    void Room::observe () {
        if (!observer_count++) {
            geo_logger.log("Activating room @%lx", (unsigned long)this);
            size_t i = 0;
            for (auto& r : residents) {
                if (r.finished) {
                    r.Resident_emerge();
                    i++;
                 }
            }
            geo_logger.log("...%lu residents", i);
        }
    }
    void Room::forget () {
        if (!observer_count) {
            geo_logger.log("Room @%lx's reference count is corrupted!", (size_t)this);
        }
        else if (!--observer_count) {
            geo_logger.log("Deactivating room @%lx", (unsigned long)this);
            size_t i = 0;
            for (auto& r : residents) {
                r.Resident_reclude();
                i++;
            }
            geo_logger.log("...and its %lu residents", i);
        }
    }

    Room* Observer::get_room () const {
        return room;
    }
    void Observer::set_room (Room* new_room) {
        if (new_room) {
            new_room->observe();
            if (observe_neighbors) {
                for (auto r : new_room->neighbors) {
                    r->observe();
                }
            }
        }
        if (room) {
            room->forget();
            if (observe_neighbors) {
                for (auto r : room->neighbors) {
                    r->forget();
                }
            }
        }
        room = new_room;
    }

    void Resident::set_room (Room* new_room) {
        Room* old_room = room;
        room = new_room;
        if (room)
            link(room->residents);
         // Make sure only to call emerge when finished
        if (finished) {
            if (room && room->observer_count) {
                if (!old_room || !old_room->observer_count) {
                    Resident_emerge();
                }
            }
            else if (old_room && old_room->observer_count) {
                Resident_reclude();
            }
        }
    }

    void Resident::finish () {
        if (!finished) {
            if (room && room->observer_count)
                Resident_emerge();
            finished = true;
        }
    }

    void Resident::reroom (Vec pos) {
        if (!room) {
            geo_logger.log("reroom was called on roomless Resident @%lx", this);
        }
        else if (!room->boundary.covers(pos)) {
            for (auto n : room->neighbors) {
                if (n->boundary.covers(pos)) {
                    set_room(n);
                    return;
                }
            }
            geo_logger.log("Resident @%lx ended up in tumbolia.", this);
            set_room(NULL);
        }
    }
    void Resident::reroom (Vec& pos) {
        if (!room) {
            geo_logger.log("reroom was called on roomless Resident @%lx", this);
        }
        else if (!room->boundary.covers(pos)) {
            for (auto n : room->neighbors) {
                if (n->boundary.covers(pos)) {
                    set_room(n);
                    return;
                }
            }
            geo_logger.log("Moving resident @%lx back into the center of the room.", this);
            pos = room->boundary.center();
        }
    }

} using namespace geo;

HACCABLE(Room) {
    name("geo::Room");
    attr("boundary", member(&Room::boundary).optional());
    attr("neighbors", member(&Room::neighbors).optional());
}

HACCABLE(Resident) {
    name("geo::Resident");
    attr("room", value_methods(&Resident::get_room, &Resident::set_room));
    finish(&Resident::finish);
}

HACCABLE(Observer) {
    name("geo::Observer");
    attr("room", value_methods(&Observer::get_room, &Observer::set_room));
}
