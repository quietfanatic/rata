
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "../inc/files.h"

namespace hacc {

void string_to_file (String s, const char* filename, write_options opts) {
    opts = hf::default_options | opts;
    FILE* outf;
    if (opts.overwrite_file()) {
        outf = fopen(filename, "w");
    }
    else if (opts.append_file()) {
        outf = fopen(filename, "a");
    }
    else {
         // Check if it exists by opening it for reading.
        outf = fopen(filename, "r");
        if (outf) {
            fclose(outf);
            throw Error("Could not create " + String(filename) + " because it already exists.");
        }
        outf = fopen(filename, "w");
    }
    if (!outf) {
        throw Error("Could not open " + String(filename) + " for writing: " + strerror(errno));
    }
    int writ = fwrite(s.data(), 1, s.length(), outf);
    if (writ < s.length()) {
        fclose(outf);
        throw Error("Could not write to " + String(filename) + ": " + strerror(errno));
    }
    if (0!=fclose(outf)) {
        throw Error("Could not close " + String(filename) + ": " + strerror(errno));
    }
    return;
}
void string_to_file (String s, String filename, write_options opts) {
    char cstr [filename.length() + 1];
    memcpy(cstr, filename.c_str(), filename.length() + 1);
    string_to_file(s, cstr, opts);
}
void file_from_string (const char* filename, String s, write_options opts) {
    string_to_file(s, filename, opts);
}
void file_from_string (String filename, String s, write_options opts) {
    string_to_file(s, filename, opts);
}



String string_from_file (const char* filename) {
    FILE* inf = fopen(filename, "r");
    if (!inf) {
        throw Error("Could not open " + String(filename) + " for reading: " + strerror(errno));
    }
     // Get filesize
    fseek(inf, 0, SEEK_END);
    long size = ftell(inf);
    fseek(inf, 0, SEEK_SET);
    char data [size];
    int read = fread(data, 1, size, inf);
    if (read < size) {
        fclose(inf);
        throw Error("Could not read from " + String(filename) + ": " + strerror(errno));
    }
    if (0!=fclose(inf)) {
        throw Error("Could not close " + String(filename) + ": " + strerror(errno));
    }
    return String(data, size);
}
String string_from_file (String filename) {
    char cstr [filename.length() + 1];
    memcpy(cstr, filename.c_str(), filename.length() + 1);
    return string_from_file(cstr);
}
String file_to_string (const char* filename) {
    return string_from_file(filename);
}
String file_to_string (String filename) {
    return string_from_file(filename);
}

}

