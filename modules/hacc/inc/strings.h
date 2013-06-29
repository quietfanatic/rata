#ifndef HAVE_HACC_STRINGS_H
#define HAVE_HACC_STRINGS_H

#include "tree.h"

namespace hacc {

     // Escape string according to HACC rules
     // Does not add quotes
    String escape_string (String);
     // Escape ident according to HACC rules
     // Does add quotes if necessary
    String escape_ident (String);
     // unescape is harder to abstract out, so we'll wait till we need it.

     // The filename parameter is used for error reporting and for serializing
     //  paths; the path's filename will be skipped if it's the current file.
    String tree_to_string (Tree*, String filename = "", uint ind = 0, uint prior_ind = 0);
    Tree* tree_from_string (String, String filename = "");
    Tree* tree_from_string (const char*, String filename = "");

    void tree_to_file (Tree*, String);
    Tree* tree_from_file (String);

    namespace X {
        struct Parse_Error : Error {
            Parse_Error (String mess = "", String filename = "", uint line = 0, uint col = 0) :
                Error(mess, filename, line, col)
            { }
        };
    }

}

#endif
