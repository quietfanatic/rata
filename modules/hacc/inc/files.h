#ifndef HAVE_HACC_FILES_H
#define HAVE_HACC_FILES_H

#include "hacc.h"


namespace hacc {

    enum {
        OVERWRITE,
        CREATE,
        APPEND
    };

    void string_to_file (String s, String filename, uint32 mode = OVERWRITE);
    void file_from_string (String filename, String s, uint32 mode = OVERWRITE);
    String string_from_file (String filename);
    String file_to_string (String filename);

}

#endif
