#ifndef HAVE_CORE_RESOURCES_H
#define HAVE_CORE_RESOURCES_H

#include "../../util/inc/honestly.h"
#include "../../hacc/inc/haccable.h"
#include "../../hacc/inc/haccable_pointers.h"
#include "commands.h"

 // The class for a resource.  Each resource has a name, and each
 //  type of resource also should have a name.

struct Resource;

struct ResourceGroup {
    Str name;
    Hash<Resource*> loaded;

    void unload_all ();
    void reload_all ();

    static Hash<ResourceGroup*>& all_groups () {
        static Hash<ResourceGroup*> r; return r;
    }
    ResourceGroup (Str name) : name(name) { all_groups().emplace(name, this); }
};

struct Resource {
    Str name;

    virtual void reload () = 0;
    virtual ~Resource () { }

    Resource (Str name) : name(name) { }
};

template <class Res, ResourceGroup* grp>
void resource_haccability () {
    hacc::Haccability<Res>::get_id([](const Res& res){ return res.name; });
    hacc::Haccability<Res>::find_by_id([](std::string id) -> Res* {
        Resource*& p = grp->loaded[id];
        if (!p) {
            try {
                p = new Res (id);
            } catch (std::exception& e) {
                fprintf(stderr, "Failed to load %s with id \"%s\": %s\n",
                    hacc::get_type_name<Res>().c_str(), id.c_str(), e.what()
                );
                grp->loaded.erase(id);
                return NULL;
            }
        }
        return static_cast<Res*>(p);
   });
}

#endif
