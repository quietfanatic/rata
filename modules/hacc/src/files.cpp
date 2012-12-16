
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "../inc/files.h"

namespace hacc {

void string_to_file (String s, Either_String filename, uint32 mode) {
    FILE* outf;
    if (mode == OVERWRITE) {
        outf = fopen(filename, "w");
    }
    else if (mode == APPEND) {
        outf = fopen(filename, "a");
    }
    else {  // mode == CREATE
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
void file_from_string (Either_String filename, String s, uint32 mode) {
    string_to_file(s, filename, mode);
}



String string_from_file (Either_String filename) {
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
String file_to_string (Either_String filename) {
    return string_from_file(filename);
}

}

