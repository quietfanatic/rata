#ifndef HAVE_GEO_EDITING_H
#define HAVE_GEO_EDITING_H

#include "../../vis/inc/common.h"

namespace geo {

    struct Resident_Editor : vis::Drawn<vis::Overlay> {
        void Drawn_draw (vis::Overlay) override;
        bool active = false;
        void activate ();
        void deactivate ();
        Resident_Editor ();
        ~Resident_Editor ();
    };
    extern Resident_Editor* resident_editor;

}

#endif
