#ifndef HAVE_HACC_STRINGS_H
#define HAVE_HACC_STRINGS_H

#include "hacc.h"

namespace hacc {

 // Escape string according to HACC rules
 // Does not add quotes
String escape_string (String);
 // Escape ident according to HACC rules
 // Does add quotes if necessary
String escape_ident (String);
 // unescape is harder to abstract out, so we'll wait till we need it.
 // Serialize the value part to a string
String hacc_value_to_string (Hacc, write_options opts = write_options(0));

 // Write a Hacc to a string
String hacc_to_string (Hacc, write_options opts = write_options(0));
 // Read a Hacc from a string
Hacc hacc_from_string (String);

}


#endif
