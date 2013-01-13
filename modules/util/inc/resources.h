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


#endif
