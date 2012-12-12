
#include "haccable.h"
#include "strings.h"
#include "files.h"


 // Objects <-> Strings
template <class C> String to_string (const C& v, write_options opts = write_options(0)) {
    return hacc_to_string(to_hacc<C>(v), opts);
}
template <class C> C from_string (String s) {
    return from_hacc<C>(hacc_from_string(s));
}
template <class C> void update_from_string (C* p, String s) {
    update_from_hacc<C>(p, hacc_from_string(s));
}
template <class C> C string_to (String s) {
    return from_string<C>(s);
}
template <class C> String string_from (const C& v, write_options opts = write_options(0)) {
    return to_string<C>(v, opts);
}
template <class C> C* new_from_string (String s) {
    return new_from_hacc<C>(hacc_from_string(s));
}
template <class C> C* string_to_new (String s) {
    return new_from_string<C>(s);
}

 // Haccs <-> Files
void hacc_to_file (Hacc h, Either_String f, write_options opts = write_options(0)) {
    string_to_file(hacc_to_string(h, opts), f, opts);
}
Hacc hacc_from_file (Either_String f) {
    return hacc_from_string(string_from_file(f));
}
void file_from_hacc (Either_String f, Hacc h, write_options opts = write_options(0)) {
    return file_from_string(f, string_from_hacc(h, opts), opts);
}
Hacc file_to_hacc (Either_String f) {
    return string_to_hacc(file_to_string(f));
}

 // Objects <-> Files
template <class C> void to_file (const C& v, Either_String f, write_options opts = write_options(0)) {
    hacc_to_file(to_hacc<C>(v), f, opts);
}
template <class C> C from_file (Either_String filename) {
    return from_hacc<C>(hacc_from_file(f));
}
template <class C> void update_from_file (C* p, Either_String f) {
    update_from_hacc<C>(p, hacc_from_file(f));
}
template <class C> C file_to (Either_String f) {
    return from_file<C>(f);
}
template <class C> void file_from (Either_String f, const C& v, write_options opts = write_options(0)) {
    to_file<C>(v, f, opts);
}
template <class C> C* new_from_file (Either_String f) {
    return new_from_hacc<C>(hacc_from_file(f));
}
template <class C> C* file_to_new (Either_String f) {
    return new_from_file<C>(f);
}

