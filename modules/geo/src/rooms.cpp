#include "geo/inc/rooms.h"

#include "hacc/inc/everything.h"
#include "util/inc/debug.h"

namespace geo {

    Room::Room () { link(&all_rooms()); }
    Room::~Room () { unlink(); }

    Links<Room>& all_rooms () {
        static Links<Room> r;
        return r;
    }

    void Room::observe (bool observe_neighbors) {
        if (!observer_count++) {
            log("geo", "Activating room @%p", this);
            size_t i = 0;
            for (auto& r : residents) {
                if (r.finished) {
                    r.Resident_emerge();
                    i++;
                }
            }
            log("geo", "...%lu residents", i);
        }
        if (observe_neighbors) {
            neighbor_observer_count++;
            for (auto n : neighbors) {
                if (n)
                    n->observe();
            }
        }
    }
    void Room::forget (bool forget_neighbors) {
        if (!observer_count) {
            log("geo", "Room @%p's reference count is corrupted!", this);
        }
        else if (!--observer_count) {
            log("geo", "Deactivating room @%p", this);
            size_t i = 0;
            for (auto& r : residents) {
                r.Resident_reclude();
                i++;
            }
            log("geo", "...and its %lu residents", i);
        }
        if (forget_neighbors && neighbor_observer_count) {
            neighbor_observer_count--;
            for (auto n : neighbors) {
                if (n)
                    n->forget();
            }
        }
    }
    void Room::finish () {
        if (neighbor_observer_count) {
            for (auto nn : neighbors) {
                if (!nn) continue;
                for (auto on : old_neighbors) {
                    if (nn == on)
                        goto next_n;
                }
                for (uint i = 0; i < neighbor_observer_count; i++) {
                    nn->observe();
                }
                next_n: { }
            }
            for (auto on : old_neighbors) {
                if (!on) continue;
                for (auto nn : neighbors) {
                    if (on == nn)
                        goto next_o;
                }
                for (uint i = 0; i < neighbor_observer_count; i++) {
                    on->forget();
                }
                next_o: { }
            }
        }
        old_neighbors = neighbors;
    }

    size_t Room::Spatial_n_pts () {
        return 4 + neighbors.size() + 1;
    }
    Vec Room::Spatial_get_pt (size_t i) {
        switch (i) {
            case 0: return Vec(0, 0);
            case 1: return Vec(boundary.r, 0);
            case 2: return boundary.rt();
            case 3: return Vec(0, boundary.t);
            default: if (i-4 < neighbors.size()) {
                if (neighbors[i-4])
                    return neighbors[i-4]->boundary.lt() - get_pos();
                else
                    return boundary.rb() + Vec(-0.5 - (i-4), 0.5) - get_pos();
            }
            else if (i-4 == neighbors.size()) {
                return boundary.rb() + Vec(-0.5 - (i-4), 0.5) - get_pos();
            }
            else {
                return Vec();
            }
        }
    }
    void Room::Spatial_set_pt (size_t i, Vec p) {
        p += get_pos();
        switch (i) {
            case 0: boundary.l = p.x; boundary.b = p.y; return;
            case 1: boundary.r = p.x; boundary.b = p.y; return;
            case 2: boundary.r = p.x; boundary.t = p.y; return;
            case 3: boundary.l = p.x; boundary.t = p.y; return;
            default: {
                if (i-4 < neighbors.size()) {
                    neighbors[i-4] = NULL;
                }
                for (auto& r : all_rooms()) {
                    if (covers(r.boundary, p)) {
                        if (i-4 < neighbors.size()) {
                            neighbors[i-4] = &r;
                        }
                        else if (i-4 == neighbors.size()) {
                            neighbors.push_back(&r);
                        }
                        break;
                    }
                }
                finish();
            }
        }
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
            log("geo", "reroom was called on roomless Resident @%lx", this);
        }
        else if (!room->boundary.covers(pos)) {
            for (auto n : room->neighbors) {
                if (n && n->boundary.covers(pos)) {
                    set_room(n);
                    return;
                }
            }
            log("geo", "Resident @%lx ended up in tumbolia.", this);
            set_room(NULL);
        }
    }
    void Resident::reroom (Vec& pos) {
        if (!room) {
            log("geo", "reroom was called on roomless Resident @%lx", this);
        }
        else if (!room->boundary.covers(pos)) {
            for (auto n : room->neighbors) {
                if (n && n->boundary.covers(pos)) {
                    set_room(n);
                    return;
                }
            }
            log("geo", "Moving resident @%lx back into the center of the room.", this);
            pos = room->boundary.center();
        }
    }

} using namespace geo;

HACCABLE(Room) {
    name("geo::Room");
    attr("boundary", member(&Room::boundary).optional());
    attr("neighbors", member(&Room::neighbors).optional());
    finish(&Room::finish);
}

HACCABLE(Resident) {
    name("geo::Resident");
    attr("room", value_methods(&Resident::get_room, &Resident::set_room).optional());
    finish(&Resident::finish);
}

HACCABLE(Observer) {
    name("geo::Observer");
    attr("room", value_methods(&Observer::get_room, &Observer::set_room).optional());
}
