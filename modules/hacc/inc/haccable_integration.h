#ifndef HAVE_HACC_HACCABLE_INTEGRATION_H
#define HAVE_HACC_HACCABLE_INTEGRATION_H


#include "haccable.h"
#include "strings.h"
#include "files.h"

namespace hacc {

 // Objects <-> Strings
template <class C> String to_string (const C& v) {
    const Hacc* h = to_hacc<C>(v);
    String r = hacc_to_string(h);
    delete h;
    return r;
}
template <class C> C value_from_string (String s) {
    const Hacc* h = hacc_from_string(s);
    const C& r = value_from_hacc<C>(h);
    delete h;
    return r;
}
template <class C> C string_to_value (String s) {
    return value_from_string<C>(s);
}
template <class C> String string_from (const C& v) {
    return to_string<C>(v);
}
template <class C> void update_from_string (C& p, String s) {
    const Hacc* h = hacc_from_string(s);
    update_from_hacc<C>(p, h);
    delete h;
}
template <class C> C* new_from_string (String s) {
    const Hacc* h = hacc_from_string(s);
    C* r = new_from_hacc<C>(h);
    delete h;
    return r;
}
template <class C> C* string_to_new (String s) {
    return new_from_string<C>(s);
}

 // Haccs <-> Files
void hacc_to_file (const Hacc* h, Either_String f) {
    string_to_file(hacc_to_string(h), f);
}
const Hacc* hacc_from_file (Either_String f) {
    return hacc_from_string(string_from_file(f));
}
void file_from_hacc (Either_String f, const Hacc* h) {
    return file_from_string(f, string_from_hacc(h));
}
const Hacc* file_to_hacc (Either_String f) {
    return string_to_hacc(file_to_string(f));
}

 // Objects <-> Files
template <class C> void to_file (const C& v, Either_String f) {
    const Hacc* h = to_hacc<C>(v);
    hacc_to_file(h, f);
    delete h;
}
template <class C> C value_from_file (Either_String f) {
    const Hacc* h = hacc_from_file(f);
    const C& r = value_from_hacc<C>(h);
    delete h;
    return r;
}
template <class C> void update_from_file (C& p, Either_String f) {
    const Hacc* h = hacc_from_file(f);
    update_from_hacc<C>(p, h);
    delete h;
}
template <class C> C file_to_value (Either_String f) {
    return value_from_file<C>(f);
}
template <class C> void file_from (Either_String f, const C& v) {
    to_file<C>(v, f);
}
template <class C> C* new_from_file (Either_String f) {
    const Hacc* h = hacc_from_file(f);
    C* r = new_from_hacc<C>(h);
    delete h;
    return r;
}
template <class C> C* file_to_new (Either_String f) {
    return new_from_file<C>(f);
}

}

#endif
