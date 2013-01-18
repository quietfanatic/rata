
#include "../inc/strings.h"
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <string.h>


namespace hacc {

String escape_string (const String& unesc) {
    String r = "";
    for (auto p = unesc.begin(); p != unesc.end(); p++) {
        switch (*p) {
            case '"': r += "\\\""; break;
            case '\\': r += "\\\\"; break;
            case '\b': r += "\\b"; break;
            case '\f': r += "\\f"; break;
            case '\n': r += "\\n"; break;
            case '\r': r += "\\r"; break;
            case '\t': r += "\\t"; break;
            default: r += String(1, *p); break;
        }
    }
    return r;
}

String escape_ident (const String& unesc) {
    if (unesc.empty()) return "\"\"";
    for (auto p = unesc.begin(); p != unesc.end(); p++) {
        if (!isalnum(*p) && *p != '_' && *p != '-')
            return "\"" + escape_string(unesc) + "\"";
    }
    return unesc;
}
String escape_id (const String& unesc) {
    String r = escape_ident(unesc);
    if (r[0] == '"') r = "&" + r;
    return r;
}


String hacc_value_to_string (const Hacc* h) {
    switch (h->form()) {
        case NULLFORM: return "null";
        case BOOL: return static_cast<const Hacc::Bool*>(h)->b ? "true" : "false";
        case INTEGER: {
            char r[32];
            sprintf(r, "%" PRIi64, static_cast<const Hacc::Integer*>(h)->i);
            return r;
        }
        case FLOAT: {
            auto fh = static_cast<const Hacc::Float*>(h);
            char r[32];
            sprintf(r, "%g~%08x", fh->f, *(uint32*)&fh->f);
            return r;
        }
        case DOUBLE: {
            char r [64];
            auto dh = static_cast<const Hacc::Double*>(h);
            sprintf(r, "%lg~%016" PRIx64, dh->d, *(uint64*)&dh->d);
            return r;
        }
        case STRING: return "\"" + escape_string(static_cast<const Hacc::String*>(h)->s) + "\"";
        case REF: return escape_id(static_cast<const Hacc::Ref*>(h)->r.id);
        case ARRAY: {
            String r = "[";
            auto& a = static_cast<const Hacc::Array*>(h)->a;
            for (auto i = a.begin(); i != a.end(); i++) {
                r += hacc_to_string(*i);
                if (i + 1 != a.end()) r += ", ";
            }
            return r + "]";
        }
        case OBJECT: {
            String r = "{";
            auto& o = static_cast<const Hacc::Object*>(h)->o;
            auto nexti = o.begin();
            for (auto i = nexti; i != o.end(); i = nexti) {
                r += escape_ident(i->first);
                r += ": ";
                r += hacc_to_string(i->second);
                nexti++;
                if (nexti != o.end()) r += ", ";
            }
            return r + "}";
        }
        case ERROR: throw static_cast<const Hacc::Error*>(h)->e;
        default: throw Error("Corrupted Hacc tree\n");
    }
}
String hacc_to_string (const Hacc* h) {
    String r = hacc_value_to_string(h);
    if (!h->id.empty()) r = escape_id(h->id) + " = " + r;
    return r;
}
String string_from_hacc (const Hacc* h) {
    return hacc_to_string(h);
}


 // Parsing is simple enough that we don't need a separate lexer step
struct Parser {
    const char* file;
    const char* begin;
    const char* p;
    const char* end;
    Parser (String s) : file(""), begin(s.data()), p(s.data()), end(s.data()+s.length()) { }
    Parser (const char* s) : file(""), begin(s), p(s), end(s + strlen(s)) { }

    int look () { return p == end ? EOF : *p; }

     // This allows us to safely use scanf, which is so much more
     // convenient than parsing numbers by hand.  Speed is not a
     // big priority here.
    const char* safebuf_begin = NULL;
    char safebuf_array [32];
    char* safebuf () {
        if (safebuf_begin != p) {
            safebuf_begin = p;
            uint len = end - p > 31 ? 31 : end - p;
            memcpy(safebuf_array, p, len);
            safebuf_array[len] = 0;
        }
        return safebuf_array;
    }

    Error error (String s) {
         // Diagnose line and column number
         // I'm not sure the col is exactly right
        uint line = 1;
        const char* nl = begin - 1;
        for (const char* p2 = begin; p2 != p; p2++) {
            if (*p2 == '\n') {
                line++;
                nl = p2;
            }
        }
        uint col = p - nl;
        return Error(s, file, line, col);
    }

     // The following are subparsers.
     // Most parsers assume the first character is already correct.
     // If a parse fails, just throw an exception, because we've got
     // datatypes with proper destructors now.

     // Utility parsers for multiple situations.

    void parse_ws () {
        while (isspace(look())) p++;
        if (p[0] == '/' && p+1 != end && p[1] == '/') {
            while (look() != '\n' && look() != EOF) p++;
            parse_ws();
        }
    }
    String parse_stringly () {
        p++;  // for the "
        String r = "";
        bool escaped = false;
        for (;;) {
            if (escaped) {
                switch (look()) {
                    case EOF: throw error("String not terminated by end of input");
                    case '"': r += '"'; break;
                    case '\\': r += "\\"; break;
                    case '/': r += "/"; break;
                    case 'b': r += "\b"; break;
                    case 'f': r += "\f"; break;
                    case 'n': r += "\n"; break;
                    case 'r': r += "\r"; break;
                    case 't': r += "\t"; break;
                    default: throw error("Unrecognized escape sequence \\" + String(p, 1));
                }
                escaped = false;
            }
            else {
                switch (look()) {
                    case EOF: throw error("String not terminated by end of input");
                    case '"': p++; return r;
                    case '\\': escaped = true; break;
                    default: r += String(p, 1);
                }
            }
            p++;
        }
    }
    String parse_ident (String what) {
        switch (look()) {
            case EOF: throw error("Expected " + what + ", but ran into the end of input");
            case '"': return parse_stringly();
            default: {
                const char* start = p;
                while (isalnum(look()) || look() == '_' || look() == '-') p++;
                if (p == start) throw error("Expected " + what + ", but saw " + String(1, look()));
                return String(start, p - start);
            }
        }
    }
    String parse_id () {
        p++;  // For the &
        return parse_ident("an identifier after &");
    }

     // Parsing of specific valtypes.
     // This one could return an int, float, or double.
    const Hacc* parse_numeric (String id) {
        int64 val;
        uint len;
        if (!sscanf(safebuf(), "%" SCNi64 "%n", &val, &len))
            throw error("Weird number");
        p += len;
        switch (look()) {
            case '~': return parse_bitrep(id);
            case '.':
            case 'e':
            case 'E':
            case 'p':  // backtrack!
            case 'P': p -= len; return parse_floating(id);
            default: return new_hacc(val, id);
        }
    }
    const Hacc* parse_floating (String id) {
        double val;
        uint len;
        if (!sscanf(safebuf(), "%lg%n", &val, &len))
            throw error("Weird number");
        p += len;
        switch (look()) {
            case '~': return parse_bitrep(id);
            default: return new_hacc(val, id);
        }
    }
    const Hacc* parse_bitrep (String id) {
        p++;  // for the ~
        uint64 rep;
        uint len;
        if (!sscanf(safebuf(), "%" SCNx64 "%n", &rep, &len))
            throw error("Missing precise bitrep after ~");
        p += len;
        switch (len) {
            case  8: return new_hacc(*(float*)&rep, id);
            case 16: return new_hacc(*(double*)&rep, id);
            default: throw error("Precise bitrep doesn't have 8 or 16 digits");
        }
    }
    const Hacc* parse_string (String id) {
        return new_hacc(parse_stringly(), id);
    }
    struct ArrayBomb {
        Array* a;
        ArrayBomb (Array* a) : a(a) { }
        void defuse () { a = null; }
        ~ArrayBomb () { if (a) for (auto p : *a) delete p; }
    };
    const Hacc* parse_array (String id) {
        Array a;
        ArrayBomb ab (&a);
        p++;  // for the [
        for (;;) {
            parse_ws();
            switch (look()) {
                case EOF: throw error("Array not terminated");
                case ':': throw error("Cannot have : in an array");
                case ',': p++; break;
                case ']': p++; {
                    ab.defuse();
                    return new_hacc(std::move(a), id);
                }
                default: a.push_back(parse_thing()); break;
            }
        }
    }
    struct ObjectBomb {
        Object* o;
        ObjectBomb (Object* o) : o(o) { }
        void defuse () { o = null; }
        ~ObjectBomb () { if (o) for (auto& p : *o) delete p.second; }
    };
    const Hacc* parse_object (String id) {
        Object o;
        ObjectBomb ob (&o);
        p++;  // for the left brace
        String key;
        for (;;) {
            parse_ws();
            switch (look()) {
                case EOF: throw error("Object not terminated");
                case ':': throw error("Missing name before : in object");
                case ',': p++; continue;
                case '}': p++; {
                    ob.defuse();
                    return new_hacc(std::move(o), id);
                }
                default: key = parse_ident("an attribute name or the end of the object"); break;
            }
            parse_ws();
            if (look() == ':') p++;
            else throw error("Missing : after name");
            parse_ws();
            switch (look()) {
                case EOF: throw error("Object not terminated");
                case ':': throw error("Extra : in object");
                case ',': throw error("Misplaced comma after : in object");
                case '}': throw error("Missing value after : in object");
                default: o.push_back(hacc_attr(key, parse_thing())); break;
            }
        }
    }
    const Hacc* parse_parens (String id) {
        p++;  // for the (
        const Hacc* r = parse_thing(id);
        parse_ws();
        if (look() == ')') p++;
        else {
            delete r;
            throw error("Extra stuff in parens");
        }
        return r;
    }
    const Hacc* parse_id_or_ref (String id) {
        String gotid;
        if (look() == '&') {
            gotid = parse_id();
        }
        else {
            gotid = parse_ident("An ID of some sort (this shouldn't happen)");
            if (gotid == "null")
                return new_hacc(null, id);
            else if (gotid == "false")
                return new_hacc(false, id);
            else if (gotid == "true")
                return new_hacc(true, id);
            else if (gotid == "nan" || gotid == "inf") {
                p -= 3;
                return parse_floating(id);
            }
        }
        parse_ws();
        if (look() == '=') {
            p++;
            return id.empty()
                ? parse_thing(gotid)
                : throw error("Too many IDs were assigned (" + id + " and " + gotid + ")");
        }
        else return new_hacc(Ref(gotid), id);
    }

    const Hacc* parse_thing (String id = "") {
        parse_ws();
        for (;;) switch (char next = look()) {
            case '+':
            case '-':
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': return parse_numeric(id);
            case '~': return parse_bitrep(id);
            case '"': return parse_string(id);
            case '[': return parse_array(id);
            case '{': return parse_object(id);
            case '(': return parse_parens(id);
            case '&': case '_': return parse_id_or_ref(id);
            default:
                if (isalnum(next))
                    return parse_id_or_ref(id);
                else throw Error("Unrecognized character " + String(1, next));
        }
    }
    const Hacc* parse_all () {
        const Hacc* r = parse_thing();
        parse_ws();
        if (look() == EOF) return r;
        else {
            delete r;
            throw error("Extra stuff at end of document");
        }
    }
    const Hacc* parse () {
        try { return parse_all(); }
        catch (Error e) { return new const Hacc::Error(e); }
    }
};

 // Finally:
const Hacc* hacc_from_string (const String& s) { return Parser(s).parse(); }
const Hacc* hacc_from_string (const char* s) { return Parser(s).parse(); }
const Hacc* string_to_hacc (const String& s) { return hacc_from_string(s); }
const Hacc* string_to_hacc (const char* s) { return hacc_from_string(s); }

}


