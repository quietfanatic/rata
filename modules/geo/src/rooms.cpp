
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

    void Room::observe (bool observe_neighbors) {
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
        if (observe_neighbors) {
            neighbor_observer_count++;
            for (auto n : neighbors) {
                n->observe();
            }
        }
    }
    void Room::forget (bool forget_neighbors) {
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
        if (forget_neighbors && neighbor_observer_count) {
            neighbor_observer_count--;
            for (auto n : neighbors) {
                n->forget();
            }
        }
    }
    void Room::set_neighbors (std::vector<Room*> new_ns) {
        for (auto nn : new_ns) {
            printf("%p\n", nn);
        }
        if (neighbor_observer_count) {
            for (auto nn : new_ns) {
                for (auto on : neighbors) {
                    if (nn == on)
                        goto next_n;
                }
                for (uint i = 0; i < neighbor_observer_count; i++) {
                    nn->observe();
                }
                next_n: { }
            }
            for (auto on : neighbors) {
                for (auto nn : new_ns) {
                    if (on == nn)
                        goto next_o;
                }
                for (uint i = 0; i < neighbor_observer_count; i++) {
                    on->forget();
                }
                next_o: { }
            }
        }
        neighbors = new_ns;
    }

    Room* Observer::get_room () const {
        return room;
    }
    void Observer::set_room (Room* new_room) {
        if (new_room) {
            new_room->observe(observe_neighbors);
        }
        if (room) {
            room->forget(observe_neighbors);
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
    attr("neighbors", value_methods(&Room::get_neighbors, &Room::set_neighbors).optional());
}

HACCABLE(Resident) {
    name("geo::Resident");
    attr("room", value_methods(&Resident::get_room, &Resident::set_room).optional());
    finish(&Resident::finish);
}

HACCABLE(Observer) {
    name("geo::Observer");
    attr("room", value_methods(&Observer::get_room, &Observer::set_room));
}
