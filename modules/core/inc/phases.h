#ifndef HAVE_CORE_PHASES_H
#define HAVE_CORE_PHASES_H

#include <string>
#include <vector>
#include "../../util/inc/honestly.h"
#include "../../util/inc/organization.h"

namespace core {

    struct Phase;
    extern std::vector<Phase*> all_phases;
    struct Phase : Ordered<Phase, all_phases> {
        std::string name;
        bool on;

        virtual void run () { }
        virtual ~Phase () { }  // This won't happen

        Phase (std::string order, std::string name = "", bool on = true) :
            Ordered(order), name(name), on(on)
        { }

        void run_if_on () { if (on) run(); }
    };

    struct Layer;
    extern std::vector<Layer*> all_layers;
    struct Layer : Ordered<Layer, all_layers> {
        std::string name;
        bool on;

        virtual void run () { }
        virtual ~Layer () { }  // This won't happen

        Layer (std::string order, std::string name = "", bool on = true) :
            Ordered(order), name(name), on(on)
        { }

        void run_if_on () { if (on) run(); }
    };

}

#endif
