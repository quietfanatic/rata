
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "../inc/files.h"

namespace hacc {

void string_to_file (String s, String filename, uint32 mode) {
    FILE* outf;
    if (mode == OVERWRITE) {
        outf = fopen(filename.c_str(), "w");
    }
    else if (mode == APPEND) {
        outf = fopen(filename.c_str(), "a");
    }
    else {  // mode == CREATE
         // Check if it exists by opening it for reading.
        outf = fopen(filename.c_str(), "r");
        if (outf) {
            fclose(outf);
            throw Error("Could not create " + filename + " because it already exists.");
        }
        outf = fopen(filename.c_str(), "w");
    }
    if (!outf) {
        throw Error("Could not open " + filename + " for writing: " + strerror(errno));
    }
    uint writ = fwrite(s.data(), 1, s.length(), outf);
    if (writ < s.length()) {
        fclose(outf);
        throw Error("Could not write to " + filename + ": " + strerror(errno));
    }
    if (0!=fclose(outf)) {
        throw Error("Could not close " + filename + ": " + strerror(errno));
    }
    return;
}
void file_from_string (String filename, String s, uint32 mode) {
    string_to_file(s, filename, mode);
}



String string_from_file (String filename) {
    FILE* inf = fopen(filename.c_str(), "r");
    if (!inf) {
        throw Error("Could not open " + filename + " for reading: " + strerror(errno));
    }
     // Get filesize
    fseek(inf, 0, SEEK_END);
    long size = ftell(inf);
    fseek(inf, 0, SEEK_SET);
    char data [size];
    int read = fread(data, 1, size, inf);
    if (read < size) {
        fclose(inf);
        throw Error("Could not read from " + filename + ": " + strerror(errno));
    }
    if (0!=fclose(inf)) {
        throw Error("Could not close " + filename + ": " + strerror(errno));
    }
    return String(data, size);
}
String file_to_string (String filename) {
    return string_from_file(filename);
}

}

