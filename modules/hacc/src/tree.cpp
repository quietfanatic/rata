#include "../inc/tree.h"

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
            case PATH: return "path";
            default: return "corrupted";
        }
    }

    Tree::~Tree () {
        switch (form) {
            case STRING: s.~String(); break;
            case ARRAY: delete a; break;
            case OBJECT: delete o; break;
            default: break;
        }
    }
    bool Tree::get_bool () const {
        if (form == BOOL) return b;
        else throw X::Logic_Error("Cannot get_bool from a " + form_name(form) + " tree.");
    }
    int64 Tree::get_integer () const {
        if (form == INTEGER) return i;
        else throw X::Logic_Error("Cannot get_integer from a " + form_name(form) + " tree.");
    }
    float Tree::get_float () const {
        switch (form) {
            case INTEGER: return i;
            case FLOAT: return f;
            case DOUBLE: return d;
            default: throw X::Logic_Error("Cannot get_float from a " + form_name(form) + " tree.");
        }
    }
    double Tree::get_double () const {
        switch (form) {
            case INTEGER: return i;
            case FLOAT: return f;
            case DOUBLE: return d;
            default: throw X::Logic_Error("Cannot get_double from a " + form_name(form) + " tree.");
        }
    }
    String Tree::get_string () const {
        if (form == STRING) return s;
        else throw X::Logic_Error("Cannot get_string from a " + form_name(form) + " tree.");
    }

    namespace X {
        Corrupted_Tree::Corrupted_Tree (Tree* tree) :
            Corrupted("Corrupted tree: nonsensical form number " + tree->form),
            tree(tree)
        { }
    }

}

