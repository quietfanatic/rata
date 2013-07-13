#ifndef HAVE_CORE_PHASES_H
#define HAVE_CORE_PHASES_H

#include <string>
#include <vector>
#include "../../util/inc/honestly.h"
#include "../../util/inc/organization.h"

namespace core {

    struct Phase;
    EXTERN_INIT_SAFE(std::vector<Phase*>, all_phases)
    struct Phase : Ordered<Phase, all_phases> {
        std::string name;
        bool on;

        virtual void Phase_start () { }
        virtual void Phase_run () { }
        virtual void Phase_stop () { }
        virtual ~Phase () { }

        Phase (std::string order, std::string name = "", bool on = true) :
            Ordered(order), name(name), on(on)
        { }

        void run_if_on () { if (on) Phase_run(); }
    };

    struct Layer;
    EXTERN_INIT_SAFE(std::vector<Layer*>, all_layers)
    struct Layer : Ordered<Layer, all_layers> {
        std::string name;
        bool on;

        virtual void Layer_start () { }
        virtual void Layer_run () { }
        virtual void Layer_stop () { }
        virtual ~Layer () { }

        Layer (std::string order, std::string name = "", bool on = true) :
            Ordered(order), name(name), on(on)
        { }

        void run_if_on () { if (on) Layer_run(); }
    };

}

#endif
