#ifndef HAVE_UTIL_RESOURCES_H
#define HAVE_UTIL_RESOURCES_H

#include "honestly.h"


 // The class for a resource.  Each resource has a name, and each
 //  type of resource also should have a name.

struct Resource {
    Str name;
    virtual bool load () = 0;
    virtual bool unload () { }
    virtual bool reload () { return unload() && load(); }
    typedef Hash<Resource*> Table;
};

 // Mixin for convenient haccability
template <class C, class Base_Haccability>
struct Resource_Haccability : Base_Haccability {
    static C* find_by_haccid (Str id) {
        C*& res = C::resource_table()[id];
        if (res) {
            return res;
        }
        else {
            res = new C;
            res->name = id;
            if (res->load()) {
                return res;
            }
            else {
                delete res;
                C::resource_table().erase(id);
                return NULL;
            }
        }
    }
    static Str haccid (const C& res) {
        return res.name;
    }
};


#endif
