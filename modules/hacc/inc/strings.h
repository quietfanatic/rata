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
String hacc_value_to_string (const Hacc*);

 // Write a Hacc to a string
String hacc_to_string (const Hacc*);
 // Read a Hacc from a string
const Hacc* hacc_from_string (const String&);
const Hacc* hacc_from_string (const char*);

String string_from_hacc (const Hacc*);
const Hacc* string_to_hacc (const String&);
const Hacc* string_to_hacc (const char*);

}


#endif
