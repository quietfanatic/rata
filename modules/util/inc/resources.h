#ifndef HAVE_UTIL_RESOURCES_H
#define HAVE_UTIL_RESOURCES_H

#include "honestly.h"


 // The class for a resource.  Each resource has a name, and each
 //  type of resource also should have a name.


struct ResourceGroup {
    Str name;
    virtual bool unload () = 0;
    virtual bool reload () = 0;
    static Hash<ResourceGroup*> all;
    ResourceGroup (Str name) : name(name) { all.emplace(name, this); }
};

HCB_BEGIN(ResourceGroup)
    get_id([](const ResourceGroup& v){ return v.name; });
    find_by_id([](String id){
        auto iter = ResourceGroup::all.find(id);
        if (iter == ResourceGroup::all.end())
            return (ResourceGroup*)NULL;
        else return iter->second;
    }
HCB_END(ResourceGroup)

template <class C>
struct Resources {
    Hash<C*> hash;
    bool unload () {
        bool success = true;
        for (auto& p : hash)
            success *= p.second->unload();
        if (success) hash.clear();
        return success;
    }
    bool reload () {
        bool success = true;
        for (auto& p : hash)
            success *= p.second->reload();
        return success;
    }
};

struct Resource {
    Str name;
    virtual bool load () = 0;
    virtual bool unload () { }
    virtual bool reload () { return unload() && load(); }
    typedef Hash<Resource*> Table;
    Resource (const ResourceGroup& grp, Str name) : name(name) {
        grp.hash.emplace(name, this);
    }
};

#endif
