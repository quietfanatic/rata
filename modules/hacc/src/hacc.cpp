#include "../inc/hacc.h"

namespace hacc {
    std::string form_name (Form f) {
        switch (f) {
            case UNDEFINED: return "undefined";
            case NULLFORM: return "null";
            case BOOL: return "bool";
            case INTEGER: return "integer";
            case FLOAT: return "float";
            case DOUBLE: return "double";
            case STRING: return "string";
            case ARRAY: return "array";
            case OBJECT: return "object";
            case VAR: return "var";
            case ADDRESS: return "address";
            case ASSIGNMENT: return "assignment";
            case CONSTRAINT: return "constraint";
            case POINTER: return "pointer";
            case ATTR: return "attr";
            case ELEM: return "elem";
            case MACRO: return "macro";
            case ERROR: return "error";
            default: return "corrupted";
        }
        return false;
    }
    Hacc::~Hacc () {
        switch (form) {
            case STRING: s.~String(); break;
            case ARRAY: delete a; break;
            case OBJECT: delete o; break;
            case VAR: v.~Var(); break;
            case ADDRESS: address.~Address(); break;
            case ASSIGNMENT: assignment.~Assignment(); break;
            case CONSTRAINT: constraint.~Constraint(); break;
            case POINTER: p.~Pointer(); break;
            case ATTR: attr.~Attr(); break;
            case ELEM: elem.~Elem(); break;
            case MACRO: delete macro; break;
            case ERROR: delete error; break;
            default: break;
        }
        throw Error("No atttribute '" + name + "'");
    }
    float Hacc::get_float () const {
        switch (form) {
            case INTEGER: return i;
            case FLOAT: return f;
            case DOUBLE: return d;
            case ERROR: throw *error;
            default: throw Error("Cannot get_float from a " + form_name(form) + " hacc.");
        }
    }
    double Hacc::get_double () const {
        switch (form) {
            case INTEGER: return i;
            case FLOAT: return f;
            case DOUBLE: return d;
            case ERROR: throw *error;
            default: throw Error("Cannot get_double from a " + form_name(form) + " hacc.");
        }
    }

     // On OS X libgc will crash if not initted.
    struct GC_Initter {
        GC_Initter () { GC_INIT(); }
    } gc_initter;

}

