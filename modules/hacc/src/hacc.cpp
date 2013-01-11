
#include "../inc/hacc.h"

namespace hacc {

 // Produce error message
const char* Error::what () const noexcept(true) {
    char ls [32]; sprintf((char*)ls, "%d", line);
    char cs [32]; sprintf((char*)cs, "%d", col);
    String r = line
        ? file.empty()
            ? mess + " at line " + (const char*)ls
                   + " col " + (const char*)cs
            : mess + " at " + file
                   + " line " + (const char*)ls
                   + " col " + (const char*)cs
        : mess;
    return r.c_str();
}

 // Get string name of a form
const char* form_name (Form form) {
    switch (form) {
        case UNDEFINED: return "undefined";
        case NULLFORM: return "null";
        case BOOL: return "bool";
        case INTEGER: return "integer";
        case FLOAT: return "float";
        case DOUBLE: return "double";
        case STRING: return "string";
        case REF: return "ref";
        case ARRAY: return "array";
        case OBJECT: return "object";
        case ERROR: return "error";
        default: return "corrupted";
    }
}

hacc::Error Hacc::form_error (hacc::String expected) const {
    if (form() == ERROR) return static_cast<const Hacc::Error*>(this)->e;
    else return Error("Expected " + expected + " Hacc, but got " + form_name(form()) + " Hacc.");
}


bool Hacc::Object::has_attr (hacc::String name) const {
    for (auto it = o.begin(); it != o.end(); it++) {
        if (it->first == name) {
            return true;
        }
    }
    return false;
}

const Hacc* Hacc::Object::attr (hacc::String name) const {
    for (auto it = o.begin(); it != o.end(); it++) {
        if (it->first == name) {
            return it->second;
        }
    }
    throw Error("No atttribute '" + name + "'");
}

#define HACC_AS_IMPL(type, name, theform) const Hacc::type* Hacc::as_##name () const { if (form() == hacc::theform) throw form_error(#name); return static_cast<const Hacc::type*>(this); }
#define HACC_GETTER_IMPL(type, name, letter) const hacc::type& Hacc::get_##name () const { return as_##name()->letter; }
#define HACC_GETTER_R_IMPL(type, name, letter) hacc::type Hacc::get_##name () const { return as_##name()->letter; }
HACC_AS_IMPL(Null, null, NULLFORM)
HACC_AS_IMPL(Bool, bool, BOOL)
HACC_AS_IMPL(Integer, integer, INTEGER)
HACC_AS_IMPL(Float, float, FLOAT)
HACC_AS_IMPL(Double, double, DOUBLE)
HACC_AS_IMPL(String, string, STRING)
HACC_AS_IMPL(Ref, ref, REF)
HACC_AS_IMPL(Array, array, ARRAY)
HACC_AS_IMPL(Object, object, OBJECT)
HACC_AS_IMPL(Error, error, ERROR)
HACC_GETTER_R_IMPL(Null, null, n)
HACC_GETTER_R_IMPL(Bool, bool, b)
HACC_GETTER_R_IMPL(Integer, integer, i)
HACC_GETTER_IMPL(String, string, s)
HACC_GETTER_IMPL(Ref, ref, r)
HACC_GETTER_IMPL(Array, array, a)
HACC_GETTER_IMPL(Object, object, o)
HACC_GETTER_IMPL(Error, error, e)
hacc::Float Hacc::get_float () const {
    switch (form()) {
        case INTEGER: return static_cast<const Hacc::Integer*>(this)->i;
        case FLOAT: return static_cast<const Hacc::Float*>(this)->f;
        case DOUBLE: return static_cast<const Hacc::Double*>(this)->d;
        default: throw form_error("numeric");
    }
}

hacc::Double Hacc::get_double () const {
    switch (form()) {
        case INTEGER: return static_cast<const Hacc::Integer*>(this)->i;
        case FLOAT: return static_cast<const Hacc::Float*>(this)->f;
        case DOUBLE: return static_cast<const Hacc::Double*>(this)->d;
        default: throw form_error("numeric");
    }
}

}

