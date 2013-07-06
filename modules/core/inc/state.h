#ifndef HAVE_CORE_STATE_H
#define HAVE_CORE_STATE_H

#include "../../util/inc/organization.h"

namespace core {

    bool load_state (std::string filename);
    bool save_state (std::string filename);

}


#endif
