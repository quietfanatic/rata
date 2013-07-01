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

     // On OS X libgc will crash if not initted.
    struct GC_Initter {
        GC_Initter () { GC_INIT(); }
    } gc_initter;

    namespace X {
        const char* Error::what () const noexcept {
            if (longmess.empty()) {
                std::stringstream ss;
                ss << mess;
                if (!filename.empty()) {
                    if (line) {
                        ss << " at " << filename << " " << line << ":" << col;
                    }
                    else {
                        ss << " while processing " << filename;
                    }
                }
                else if (line) {
                    ss << " at " << line << ":" << col;
                }
                longmess = ss.str();
            }
            return longmess.c_str();
        }

        Corrupted_Tree::Corrupted_Tree (Tree* tree) :
            Corrupted("Corrupted tree: nonsensical form number " + tree->form),
            tree(tree)
        { }
        static String combine_messes (const std::vector<std::exception_ptr>& errs) {
            String r = "Combo Error: [\n";
             // This is kind of a dumb way to mix error messages, but it's the
             //  only way to do it without requiring boilerplate everywhere.
            for (auto e : errs) {
                try { std::rethrow_exception(e); }
                catch (std::exception& e) {
                    r += "    " + String(e.what()) + "\n";
                }
            }
            return r + "]";
        }
        Combo_Error::Combo_Error (const std::vector<std::exception_ptr>& errs) :
            Error(combine_messes(errs)), errs(errs)
        { }
        Combo_Error::Combo_Error (std::vector<std::exception_ptr>&& errs) :
            Error(combine_messes(errs)), errs(errs)
        { }
    }

}

