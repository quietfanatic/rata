
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

static String indent (uint n) {
    String r = "";
    for (; n; n--) r += "    ";
    return r;
}
static String hacc_to_string_b (Hacc* h, uint ind, uint prior_ind, bool sp) {
    String r = hacc_value_to_string(h, ind, prior_ind);
    if (!h->id.empty()) {
        r = "$" + escape_ident(h->id) + " = " + r;
        if (sp) r = " " + r;
    }
    return r;
}

String hacc_value_to_string (Hacc* h, uint ind, uint prior_ind) {
    switch (h->form()) {
        case NULLFORM: return "null";
        case BOOL: return static_cast<Hacc::Bool*>(h)->b ? "true" : "false";
        case INTEGER: {
            char r[32];
            sprintf(r, "%" PRIi64, static_cast<Hacc::Integer*>(h)->i);
            return r;
        }
        case FLOAT: {
            auto fh = static_cast<Hacc::Float*>(h);
            char r[32];
            sprintf(r, "%g~%08x", fh->f, *(uint32*)&fh->f);
            return r;
        }
        case DOUBLE: {
            char r [64];
            auto dh = static_cast<Hacc::Double*>(h);
            sprintf(r, "%lg~%016" PRIx64, dh->d, *(uint64*)&dh->d);
            return r;
        }
        case STRING: return "\"" + escape_string(static_cast<Hacc::String*>(h)->s) + "\"";
        case REF: return "$" + escape_ident(static_cast<Hacc::Ref*>(h)->r.id);
        case ATTRREF: {
            auto arh = static_cast<Hacc::AttrRef*>(h);
            return hacc_to_string(arh->ar.subject) + "." + escape_ident(arh->ar.name);
        }
        case ELEMREF: {
            auto erh = static_cast<Hacc::ElemRef*>(h);
            char is[32];
            sprintf(is, "%ld", erh->er.index);
            return hacc_to_string(erh->er.subject) + "." + is;
        }
        case DEREF: {
            return hacc_to_string(static_cast<Hacc::DeRef*>(h)->dr.subject) + ".^";
        }
        case MACROCALL: {
            auto mch = static_cast<Hacc::MacroCall*>(h);
            return mch->mc.name + "(" + hacc_to_string(mch->mc.arg) + ")";
        }
        case ARRAY: {
            auto& a = static_cast<Hacc::Array*>(h)->a;
            if (a.size() == 0) return "[]";
            String r = "[";
            if (ind && a.size() > 1)
                r += "\n" + indent(prior_ind + 1);
            for (auto i = a.begin(); i != a.end(); i++) {
                if (ind)
                    r += hacc_to_string(*i, ind - 1, prior_ind + (a.size() > 1));
                else r += hacc_to_string(*i);
                if (i + 1 != a.end()) {
                    if (ind && a.size() > 1)
                        r += "\n" + indent(prior_ind + 1);
                    else r += " ";
                }
            }
            if (ind && a.size() > 1)
                r += "\n" + indent(prior_ind);
            return r + "]";
        }
        case OBJECT: {
            auto& o = static_cast<Hacc::Object*>(h)->o;
            if (o.size() == 0) return "{}";
            String r = "{";
            if (ind && o.size() > 1)
                r += "\n" + indent(prior_ind + 1);
            else r += " ";
            auto nexti = o.begin();
            for (auto i = nexti; i != o.end(); i = nexti) {
                r += escape_ident(i->first);
                r += ":";
                if (ind)
                    r += hacc_to_string_b(i->second, ind - 1, prior_ind + (o.size() > 1), true);
                else r += hacc_to_string_b(i->second, 0, 0, true);
                nexti++;
                if (nexti != o.end()) {
                    if (ind) r += "\n" + indent(prior_ind + 1);
                    else r += " ";
                }
            }
            if (ind && o.size() > 1)
                r += "\n" + indent(prior_ind);
            else r += " ";
            return r + "}";
        }
        case GENERIC: throw Error("Cannot stringify a Generic.  This may be an internal error.");
        case ERROR: throw static_cast<Hacc::Error*>(h)->e;
        default: throw Error("Corrupted Hacc tree\n");
    }
}
String hacc_to_string (Hacc* h, uint ind, uint prior_ind) {
    return hacc_to_string_b(h, ind, prior_ind, false);
}
String string_from_hacc (Hacc* h, uint ind, uint prior_ind) {
    return hacc_to_string(h, ind, prior_ind);
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
    Hacc* parse_numeric () {
        int64 val;
        uint len;
        if (!sscanf(safebuf(), "%" SCNi64 "%n", &val, &len))
            return parse_floating();
        p += len;
        switch (look()) {
            case '~': return parse_bitrep();
            case '.':
            case 'e':
            case 'E':
            case 'p':  // backtrack!
            case 'P': p -= len; return parse_floating();
            default: return new_hacc(val);
        }
    }
    Hacc* parse_floating () {
        double val;
        uint len;
        if (!sscanf(safebuf(), "%lg%n", &val, &len))
            throw error("Weird number");
        p += len;
        switch (look()) {
            case '~': return parse_bitrep();
            default: return new_hacc(val);
        }
    }
    Hacc* parse_bitrep () {
        p++;  // for the ~
        uint64 rep;
        uint len;
        if (!sscanf(safebuf(), "%" SCNx64 "%n", &rep, &len))
            throw error("Missing precise bitrep after ~");
        p += len;
        switch (len) {
            case  8: return new_hacc(*(float*)&rep);
            case 16: return new_hacc(*(double*)&rep);
            default: throw error("Precise bitrep doesn't have 8 or 16 digits");
        }
    }
    Hacc* parse_string () {
        return new_hacc(parse_stringly());
    }
    Hacc* parse_heredoc () {
        p++;  // for the <
        if (look() != '<') throw error("< isn't followed by another < for a heredoc");
        p++;
        String terminator = parse_ident("heredoc delimiter string after <<");
        while (look() == ' ' || look() == '\t') p++;
        if (look() != '\n') throw error("Extra stuff after <<" + terminator + " before end of line");
        p++;
        String got = "";
        while (1) {
            String ind = "";
            while (look() == ' ' || look() == '\t') {
                ind += look(); got += look(); p++;
            }
            if (p + terminator.size() > end) throw error("Ran into end of document before + terminator");
            if (0==strncmp(p, terminator.c_str(), terminator.size())) {
                String ret;
                size_t p1 = 0;
                size_t p2 = got.find('\n');
                while (p2 != String::npos) {
                    p2 += 1;
                    if (0==strncmp(got.c_str() + p1, ind.c_str(), ind.size())) {
                        p1 += ind.size();
                    }
                    ret += got.substr(p1, p2 - p1);
                    p1 = p2;
                    p2 = got.find('\n', p2);
                }
                p += terminator.size();
                return new_hacc(ret);
            }
            while (look() != '\n') {
                got += look(); p++;
            }
            got += look(); p++;
        }
    }
    Hacc* parse_array () {
        Array a;
        p++;  // for the [
        for (;;) {
            parse_ws();
            switch (look()) {
                case EOF: throw error("Array not terminated");
                case ':': throw error("Cannot have : in an array");
                case ',': p++; break;
                case ']': p++; {
                    return new_hacc(std::move(a));
                }
                default: a.push_back(parse_thing()); break;
            }
        }
    }
    Hacc* parse_object () {
        Object o;
        p++;  // for the {
        String key;
        for (;;) {
            parse_ws();
            switch (look()) {
                case EOF: throw error("Object not terminated");
                case ':': throw error("Missing name before : in object");
                case ',': p++; continue;
                case '}': p++; {
                    return new_hacc(std::move(o));
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
    Hacc* parse_parens () {
        p++;  // for the (
        Hacc* r = parse_thing();
        parse_ws();
        if (look() == ')') p++;
        else {
            delete r;
            throw error("Extra stuff in parens");
        }
        return r;
    }
    Hacc* parse_var (String type) {
        p++;  // for the $
        String gotid = parse_ident("An variable name after $");
        parse_ws();
        if (look() == '=') {
            p++;
            return parse_thing(type, gotid);
        }
        else return new_hacc(Ref(gotid));
    }
    Hacc* parse_bareword () {
        String word = parse_ident("An ID of some sort (this shouldn't happen)");
        if (word == "null")
            return new_hacc(null);
        else if (word == "false")
            return new_hacc(false);
        else if (word == "true")
            return new_hacc(true);
        else if (word == "nan" || word == "inf") {
            p -= 3;
            return parse_floating();
        }
        else if (look() == '(') {
            return new_hacc(hacc::MacroCall(word, parse_parens()));
        }
        else {
            return new_hacc(word);
        }
    }
    Hacc* parse_derefs (Hacc* subject, String type, String id) {
        if (look() == '.') {
            p++;
            if (look() == '^') {
                p++;
                return parse_derefs(new_hacc(hacc::DeRef(subject)), type, id);
            }
            if (isdigit(look())) {
                uint64 index;
                uint len;
                if (!sscanf(safebuf(), "%" SCNu64 "%n", &index, &len))
                    throw error("Weird number in element dereference");
                p += len;
                return parse_derefs(new_hacc(hacc::ElemRef(subject, index)), type, id);
            }
            else if (isalpha(look()) || look() == '_' || look() == '"') {
                String name = parse_ident("attribute name");
                return parse_derefs(new_hacc(hacc::AttrRef(subject, name)), type, id);
            }
            else throw error("dot not followed by an ident or number");
        }
        else {
            if (!type.empty()) {
                const_cast<Hacc*>(subject)->type = type;
            }
            if (!id.empty()) {
                if (!subject->id.empty())
                    throw error("Too many IDs given");
                const_cast<Hacc*>(subject)->id = id;
            }
            return subject;
        }
    }

    Hacc* parse_typed_thing (String type, String id) {
        if (!type.empty())
            throw error("Too many #types given");
        p++;  // for the #
        return parse_thing(parse_ident("type after '#'"), id);
    }

    Hacc* parse_thing (String type = "", String id = "") {
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
            case '9': return parse_derefs(parse_numeric(), type, id);
            case '#': return parse_typed_thing(type, id);
            case '~': return parse_derefs(parse_bitrep(), type, id);
            case '"': return parse_derefs(parse_string(), type, id);
            case '[': return parse_derefs(parse_array(), type, id);
            case '{': return parse_derefs(parse_object(), type, id);
            case '(': return parse_derefs(parse_parens(), type, id);
            case '$': return parse_derefs(parse_var(type), type, id);
            case '_': return parse_derefs(parse_bareword(), type, id);
            case '<': return parse_derefs(parse_heredoc(), type, id);
            default:
                if (isalnum(next))
                    return parse_derefs(parse_bareword(), type, id);
                else throw Error("Unrecognized character " + String(1, next));
        }
    }
    Hacc* parse_all () {
        Hacc* r = parse_thing();
        parse_ws();
        if (look() == EOF) return r;
        else {
            delete r;
            throw error("Extra stuff at end of document");
        }
    }
    Hacc* parse () {
        try { return parse_all(); }
        catch (Error e) { return new Hacc::Error(e); }
    }
};

 // Finally:
Hacc* hacc_from_string (const String& s) { return Parser(s).parse(); }
Hacc* hacc_from_string (const char* s) { return Parser(s).parse(); }
Hacc* string_to_hacc (const String& s) { return hacc_from_string(s); }
Hacc* string_to_hacc (const char* s) { return hacc_from_string(s); }

}


