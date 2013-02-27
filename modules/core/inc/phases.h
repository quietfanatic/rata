#ifndef HAVE_CORE_PHASES_H
#define HAVE_CORE_PHASES_H

#include <string>
#include <vector>
#include "../../util/inc/honestly.h"
#include "../../util/inc/organization.h"

namespace core {

struct Phase : Ordered<Phase> {
    std::string order;
    std::string name;
    bool on;

    virtual void run () { }
    virtual ~Phase () { }  // This won't happen

    Phase (std::string order, std::string name = "", bool on = true) :
        Ordered<Phase>(order), order(order), name(name), on(on)
    { }

    void run_if_on () { if (on) run(); }
};
struct Layer : Ordered<Layer> {
    std::string order;
    std::string name;
    bool on;

    virtual void run () { }
    virtual ~Layer () { }  // This won't happen

    Layer (std::string order, std::string name = "", bool on = true) :
        Ordered<Layer>(order), order(order), name(name), on(on)
    { }

    void run_if_on () { if (on) run(); }
};

}

#endif
