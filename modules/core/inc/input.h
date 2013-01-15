#ifndef HAVE_CORE_INPUT_H
#define HAVE_CORE_INPUT_H

#include "../../util/inc/honestly.h"

namespace core {
    void init_input ();
    void read_input ();
    uint8 get_key (uint code);
}

#endif
