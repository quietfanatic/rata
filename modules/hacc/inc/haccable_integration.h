#ifndef HAVE_HACC_HACCABLE_INTEGRATION_H
#define HAVE_HACC_HACCABLE_INTEGRATION_H


#include "haccable.h"
#include "strings.h"
#include "files.h"

namespace hacc {

 // Objects <-> Strings
template <class C> String to_string (const C& v) {
    return hacc_to_string(to_hacc<C>(v));
}
template <class C> C value_from_string (String s) {
    return std::move(value_from_hacc<C>(hacc_from_string(s)));
}
template <class C> C string_to_value (String s) {
    return value_from_string<C>(s);
}
template <class C> String string_from (const C& v) {
    return to_string<C>(v);
}
template <class C> void update_from_string (C& p, String s) {
    update_from_hacc<C>(p, hacc_from_string(s));
}
template <class C> C* new_from_string (String s) {
    return new_from_hacc<C>(hacc_from_string(s));
}
template <class C> C* string_to_new (String s) {
    return new_from_string<C>(s);
}

 // Haccs <-> Files
static inline void hacc_to_file (Hacc* h, Either_String f) {
    string_to_file(hacc_to_string(h), f);
}
static inline Hacc* hacc_from_file (Either_String f) {
    return hacc_from_string(string_from_file(f));
}
static inline void file_from_hacc (Either_String f, Hacc* h) {
    return file_from_string(f, string_from_hacc(h));
}
static inline Hacc* file_to_hacc (Either_String f) {
    return string_to_hacc(file_to_string(f));
}

 // Objects <-> Files
template <class C> void to_file (const C& v, Either_String f) {
    hacc_to_file(to_hacc(v), f);
}
template <class C> C value_from_file (Either_String f) {
    return value_from_hacc<C>(hacc_from_file(f));
}
template <class C> void update_from_file (C& p, Either_String f) {
    update_from_hacc<C>(p, hacc_from_file(f));
}
template <class C> C file_to_value (Either_String f) {
    return value_from_file<C>(f);
}
template <class C> void file_from (Either_String f, const C& v) {
    to_file<C>(v, f);
}
template <class C> C* new_from_file (Either_String f) {
    return new_from_hacc<C>(hacc_from_file(f));
}
template <class C> C* file_to_new (Either_String f) {
    return new_from_file<C>(f);
}

}

#endif
