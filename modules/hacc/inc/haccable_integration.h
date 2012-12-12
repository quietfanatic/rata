
#include "haccable.h"
#include "strings.h"

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
