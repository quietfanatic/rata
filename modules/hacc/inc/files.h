#ifndef HAVE_HACC_FILES_H
#define HAVE_HACC_FILES_H

#include "hacc.h"

namespace hacc {

void string_to_file (String s, String filename, write_options opts = write_options(0));
void string_to_file (String s, const char* filename, write_options opts = write_options(0));
void file_from_string (String filename, String s, write_options opts = write_options(0));
void file_from_string (const char* filename, String s, write_options opts = write_options(0));
String string_from_file (String filename);
String string_from_file (const char* filename);
String file_to_string (String filename);
String file_to_string (const char* filename);

}

#endif
