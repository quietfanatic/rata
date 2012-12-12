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

void string_to_file (String s, Either_String filename, write_options opts = write_options(0));
void file_from_string (Either_String filename, String s, write_options opts = write_options(0));
String string_from_file (Either_String filename);
String file_to_string (Either_String filename);

}

#endif
