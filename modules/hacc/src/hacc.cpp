
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
        case POINTER: return "pointer";
        case ARRAY: return "array";
        case OBJECT: return "object";
        case ERROR: return "error";
        default: return "corrupted";
    }
}
Error Hacc::form_error (String expected) const {
    if (value.form == ERROR) return value.e;
    else return Error("This Hacc is not " + expected + ", it's of form " + form_name(value.form));
}


Value::~Value () {
    switch (form) {
        case STRING: s.~String(); return;
        case POINTER: p.~Pointer(); return;
        case ARRAY: a.~Array(); return;
        case OBJECT: o.~Object(); return;
        case ERROR: e.~Error(); return;
        default: break;
    }
}
template <int size> struct storage {
    void* aligner [size / sizeof(void*) + (size % sizeof(void*) != 0)];
};
Value::Value (Value&& rv) {
    *((storage<sizeof(Value)>*)this)
     =
    *((storage<sizeof(Value)>*)&rv);
    rv.form = UNDEFINED;
}
void Value::set (Value&& rv) {
    this->~Value();
    *((storage<sizeof(Value)>*)this)
     =
    *((storage<sizeof(Value)>*)&rv);
    rv.form = UNDEFINED;
}

Null    Hacc::get_null    () const { if (value.form == NULLFORM) return value.n; else throw form_error("a null"); }
Bool    Hacc::get_bool    () const { if (value.form == BOOL) return value.b; else throw form_error("a bool"); }
Integer Hacc::get_integer () const { if (value.form == INTEGER) return value.i; else throw form_error("an integer"); }
Float   Hacc::get_float   () const {
    switch (value.form) {
         // Automatic conversion
        case INTEGER: return value.i;
        case FLOAT:   return value.f;
        case DOUBLE:  return value.d;
        default: throw form_error("a number");
    }
}
Double  Hacc::get_double () const {
    switch (value.form) {
         // Automatic conversion
        case INTEGER: return value.i;
        case FLOAT:   return value.f;
        case DOUBLE:  return value.d;
        default: throw form_error("a number");
    }
}
const String&  Hacc::get_string  () const { if (value.form == STRING) return value.s; else throw form_error("a string"); }
const Pointer& Hacc::get_pointer () const { if (value.form == POINTER) return value.p; else throw form_error("a pointer"); }
const Array&   Hacc::get_array   () const { if (value.form == ARRAY) return value.a; else throw form_error("an array"); }
const Object&  Hacc::get_object  () const { if (value.form == OBJECT) return value.o; else throw form_error("an object"); }

Hacc& Hacc::get_elem (uint i) const {
    if (value.form != ARRAY) throw form_error("an array");
    if (i >= value.a.size()) throw Error("Index out of range");
    return *value.a[i];
}

bool Hacc::has_attr (String name) const {
    if (value.form != OBJECT) return false;
    for (auto it = value.o.begin(); it != value.o.end(); it++) {
        if (it->first == name) {
            return true;
         }
    }
    return false;
}

Hacc& Hacc::get_attr (String name) const {
    if (value.form != OBJECT) throw form_error("an object");
    for (auto it = value.o.begin(); it != value.o.end(); it++) {
        if (it->first == name) {
            return *it->second;
        }
    }
    throw Error("No element '" + name + "'");
}

void Hacc::add_elem (Hacc&& h) {
    if (value.form != ARRAY) throw form_error("an array");
    value.a.push_back(std::unique_ptr<Hacc>(new Hacc (std::forward<Hacc>(h))));
}
void Hacc::add_attr (String name, Hacc&& h) {
    if (value.form != OBJECT) throw form_error("an object");
    value.o.push_back(Pair<std::unique_ptr<Hacc>>(name, std::unique_ptr<Hacc>(new Hacc (std::forward<Hacc>(h)))));
}


}


