#ifndef HAVE_HACC_DOCUMENTS_H
#define HAVE_HACC_DOCUMENTS_H

#include "hacc.h"

namespace hacc {
     // Autoloads
    Pointer doc (std::string);
    void load (const std::vector<std::string>&);
    void unload (const std::vector<std::string>&);
     // Fails if something references it
    void safe_unload (const std::vector<std::string>&);
    void reload (const std::vector<std::string>&);
}

#endif
