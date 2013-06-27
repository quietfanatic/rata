#ifndef HAVE_HACC_STRINGS_H
#define HAVE_HACC_STRINGS_H

#include "hacc.h"

namespace hacc {

     // Escape string according to HACC rules
     // Does not add quotes
    String escape_string (const String&);
     // Escape ident according to HACC rules
     // Does add quotes if necessary
    String escape_ident (const String&);
     // unescape is harder to abstract out, so we'll wait till we need it.

     // Write a Hacc to a string
    String hacc_to_string (Hacc*, uint ind = 0, uint prior_ind = 0);
     // Read a Hacc from a string
    Hacc* hacc_from_string (const String&, String filename = "");
    Hacc* hacc_from_string (const char*, String filename = "");

    void hacc_to_file (Hacc*, const String&, uint in = 0, uint prior_ind = 0);
    void hacc_from_file (const String&);
}


#endif
