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

    bool unload ();
    bool reload ();

    static Hash<ResourceGroup*>& all_groups () {
        static Hash<ResourceGroup*> r; return r;
    }
    ResourceGroup (Str name) : name(name) { all_groups().emplace(name, this); }
};

struct Resource {
    Str name;

    virtual bool unload () { return false; }
    virtual bool reload () { return false; }
    virtual ~Resource () { }

    Resource (Str name) : name(name) { }
};

#endif
