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
 // Serialize the value part to a string
String hacc_value_to_string (Hacc*);

 // Write a Hacc to a string
String hacc_to_string (Hacc*);
 // Read a Hacc from a string
Hacc* hacc_from_string (const String&);
Hacc* hacc_from_string (const char*);

String string_from_hacc (Hacc*);
Hacc* string_to_hacc (const String&);
Hacc* string_to_hacc (const char*);

}


#endif
