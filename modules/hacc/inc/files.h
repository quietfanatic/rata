#ifndef HAVE_HACC_FILES_H
#define HAVE_HACC_FILES_H

#include "hacc.h"


namespace hacc {

 // This must be no less ephemeral than the pointer returned by std::string::c_str()
struct Either_String {
    const char* c_str;
    constexpr Either_String (const char* c_str) : c_str(c_str) { }
    Either_String (hacc::String s) : c_str(s.c_str()) { }
    constexpr operator const char* () { return c_str; }
};

enum {
    OVERWRITE,
    CREATE,
    APPEND
};

void string_to_file (String s, Either_String filename, uint32 mode = OVERWRITE);
void file_from_string (Either_String filename, String s, uint32 mode = OVERWRITE);
String string_from_file (Either_String filename);
String file_to_string (Either_String filename);

}

#endif
