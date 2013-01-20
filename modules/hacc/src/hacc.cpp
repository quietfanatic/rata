
#include "../inc/hacc.h"
#include "../inc/haccable.h"
#include "../inc/haccable_files.h"

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
        case GENERIC: return "generic";
        case ATTRREF: return "attrref";
        case ELEMREF: return "elemref";
        case MACROCALL: return "macrocall";
        case ARRAY: return "array";
        case OBJECT: return "object";
        case ERROR: return "error";
        default: return "corrupted";
    }
}

hacc::Error Hacc::form_error (hacc::String expected) {
    if (form() == ERROR) return static_cast<Hacc::Error*>(this)->e;
    else return Error("Expected " + expected + " Hacc, but got " + form_name(form()) + " Hacc.");
}


bool Hacc::Object::has_attr (hacc::String name) const  {
    for (auto it = o.begin(); it != o.end(); it++) {
        if (it->first == name) {
            return true;
        }
    }
    return false;
}

Hacc* Hacc::Object::attr (hacc::String name) const {
    for (auto it = o.begin(); it != o.end(); it++) {
        if (it->first == name) {
            return it->second;
        }
    }
    fprintf(stderr, "No atttribute '%s' found.\n", name.c_str());
    throw Error("No atttribute '" + name + "'");
}

#define HACC_AS_IMPL(type, name, theform) Hacc::type* Hacc::as_##name () { if (form() != hacc::theform) throw form_error(#name); return static_cast<Hacc::type*>(this); }
#define HACC_GETTER_IMPL(type, name, letter) hacc::type& Hacc::get_##name () { return as_##name()->letter; }
#define HACC_GETTER_R_IMPL(type, name, letter) hacc::type Hacc::get_##name () { return as_##name()->letter; }
HACC_AS_IMPL(Null, null, NULLFORM)
HACC_AS_IMPL(Bool, bool, BOOL)
HACC_AS_IMPL(Integer, integer, INTEGER)
HACC_AS_IMPL(Float, float, FLOAT)
HACC_AS_IMPL(Double, double, DOUBLE)
HACC_AS_IMPL(String, string, STRING)
HACC_AS_IMPL(Ref, ref, REF)
HACC_AS_IMPL(Generic, generic, GENERIC)
HACC_AS_IMPL(AttrRef, attrref, ATTRREF)
HACC_AS_IMPL(ElemRef, elemref, ELEMREF)
HACC_AS_IMPL(MacroCall, macrocall, MACROCALL)
HACC_AS_IMPL(Array, array, ARRAY)
HACC_AS_IMPL(Object, object, OBJECT)
HACC_AS_IMPL(Error, error, ERROR)
HACC_GETTER_R_IMPL(Null, null, n)
HACC_GETTER_R_IMPL(Bool, bool, b)
HACC_GETTER_R_IMPL(Integer, integer, i)
HACC_GETTER_IMPL(String, string, s)
HACC_GETTER_IMPL(Ref, ref, r)
HACC_GETTER_IMPL(Generic, generic, g)
HACC_GETTER_IMPL(AttrRef, attrref, ar)
HACC_GETTER_IMPL(ElemRef, elemref, er)
HACC_GETTER_IMPL(MacroCall, macrocall, mc)
HACC_GETTER_IMPL(Array, array, a)
HACC_GETTER_IMPL(Object, object, o)
HACC_GETTER_IMPL(Error, error, e)
hacc::Float Hacc::get_float () {
    switch (form()) {
        case INTEGER: return static_cast<Hacc::Integer*>(this)->i;
        case FLOAT: return static_cast<Hacc::Float*>(this)->f;
        case DOUBLE: return static_cast<Hacc::Double*>(this)->d;
        default: throw form_error("numeric");
    }
}

hacc::Double Hacc::get_double () {
    switch (form()) {
        case INTEGER: return static_cast<Hacc::Integer*>(this)->i;
        case FLOAT: return static_cast<Hacc::Float*>(this)->f;
        case DOUBLE: return static_cast<Hacc::Double*>(this)->d;
        default: throw form_error("numeric");
    }
}


Hacc* collapse_hacc (Hacc* h) {
    Hacc* newh;
    switch (h->form()) {
        case MACROCALL: {
            auto mch = static_cast<hacc::Hacc::MacroCall*>(h);
            if (mch->mc.name == "file") {
                if (mch->mc.arg->form() == STRING) {
                    newh = new_hacc(generic_from_file(mch->mc.arg->get_string()));
                }
                else throw Error("The \"file\" macro can only be called on a string.\n");
            }
            else {
                throw Error("Unrecognized macro \"" + mch->mc.name + "\" (only \"file\" currently exists)\n");
            }
            break;
        }
        case ATTRREF: {
            auto arh = static_cast<hacc::Hacc::AttrRef*>(h);
            arh->ar.subject = collapse_hacc(arh->ar.subject);
            if (arh->ar.subject->form() == GENERIC) {
                auto gh = static_cast<hacc::Hacc::Generic*>(h);
                HaccTable* t = HaccTable::require_cpptype(*gh->g.cpptype);
                newh = new_hacc(t->get_attr(gh->g.p, arh->ar.name));
            }
            else throw Error("Attributes can only be requested from a \"Generic\" hacc, such as produced by file()\n");
        }
        case ELEMREF: {
            auto erh = static_cast<hacc::Hacc::ElemRef*>(h);
            erh->er.subject = collapse_hacc(erh->er.subject);
            if (erh->er.subject->form() == GENERIC) {
                auto gh = static_cast<hacc::Hacc::Generic*>(h);
                HaccTable* t = HaccTable::require_cpptype(*gh->g.cpptype);
                newh = new_hacc(t->get_elem(gh->g.p, erh->er.index));
            }
            else throw Error("Elements can only be requested from a \"Generic\" hacc, such as produced by file()\n");
        }
        default: return h;
    }
    if (newh != h) {
        h->destroy();
        delete h;
    }
    return newh;
}


}

