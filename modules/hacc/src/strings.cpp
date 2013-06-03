
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <string.h>
#include "../inc/strings.h"

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

String hacc_to_string (Hacc* h, uint ind, uint prior_ind) {
    switch (h->form) {
        case NULLFORM: return "null";
        case BOOL: return h->b ? "true" : "false";
        case INTEGER: {
            std::ostringstream s;
            s << h->i;
            return s.str();
        }
        case FLOAT: {
            std::ostringstream s;
            s << h->f << "~" << std::hex << *(uint32*)&h->f;
            return s.str();
        }
        case DOUBLE: {
            std::ostringstream s;
            s << h->d << "~" << std::hex << *(uint64*)&h->d;
            return s.str();
        }
        case STRING: return "\"" + escape_string(h->s) + "\"";
        case VAR: return "$" + escape_ident(h->v.name);
        case ASSIGNMENT: {
            return hacc_to_string(h->assignment.left) + "=" + hacc_to_string(h->assignment.right);
        }
        case CONSTRAINT: {
            return "#" + escape_ident(h->constraint.hacctype) + "(" + hacc_to_string(h->constraint.value) + ")";
        }
        case ATTR: {
            return hacc_to_string(h->attr.subject) + "." + escape_ident(h->attr.name);
        }
        case ELEM: {
            String e;
            std::stringstream(e) << h->elem.index;
            return hacc_to_string(h->elem.subject) + "." + e;
        }
        case ADDRESS: {
            return "&" + hacc_to_string(h->address.subject);
        }
        case MACRO: {
            String r = h->macro->name + "(";
            for (auto& arg : h->macro->args) {
                r += hacc_to_string(arg);
                if (&arg != &h->macro->args.back())
                    r += " ";
            };
            return r + ")";
        }
        case ARRAY: {
            if (h->a->size() == 0) return "[]";
            String r = "[";
            if (ind && h->a->size() > 1)
                r += "\n" + indent(prior_ind + 1);
            for (auto& e : *h->a) {
                if (ind)
                    r += hacc_to_string(e, ind - 1, prior_ind + (h->a->size() > 1));
                else r += hacc_to_string(e);
                if (&e != &h->a->back()) {
                    if (ind && h->a->size() > 1)
                        r += "\n" + indent(prior_ind + 1);
                    else r += " ";
                }
            }
            if (ind && h->a->size() > 1)
                r += "\n" + indent(prior_ind);
            return r + "]";
        }
        case OBJECT: {
            if (h->o->size() == 0) return "{}";
            String r = "{";
            if (ind && h->o->size() > 1)
                r += "\n" + indent(prior_ind + 1);
            else r += " ";
            auto nexti = h->o->begin();
            for (auto i = nexti; i != h->o->end(); i = nexti) {
                r += escape_ident(i->first);
                r += ":";
                if (ind)
                    r += hacc_to_string(i->second, ind - 1, prior_ind + (h->o->size() > 1));
                else r += hacc_to_string(i->second, 0, 0);
                nexti++;
                if (nexti != h->o->end()) {
                    if (ind) r += "\n" + indent(prior_ind + 1);
                    else r += " ";
                }
            }
            if (ind && h->o->size() > 1)
                r += "\n" + indent(prior_ind);
            else r += " ";
            return r + "}";
        }
        case POINTER: {
            return "<pointer>";
        }
        case ERROR: throw *h->error;
        default: throw Error("Corrupted Hacc tree\n");
    }
}
String string_from_hacc (Hacc* h, uint ind, uint prior_ind) {
    return hacc_to_string(h, ind, prior_ind);
}

 // Parsing is simple enough that we don't need a separate lexer step
 // TODO: refactor with a lexer step T_T
struct Parser {
    String file;
    const char* begin;
    const char* p;
    const char* end;
    Parser (String s, String file = "") : file(file), begin(s.data()), p(s.data()), end(s.data()+s.length()) { }
    Parser (const char* s, String file = "") : file(file), begin(s), p(s), end(s + strlen(s)) { }

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
            default: return new Hacc(val);
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
            default: return new Hacc(val);
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
            case  8: return new Hacc(*(float*)&rep);
            case 16: return new Hacc(*(double*)&rep);
            default: throw error("Precise bitrep doesn't have 8 or 16 digits");
        }
    }
    Hacc* parse_string () {
        return new Hacc(parse_stringly());
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
                return new Hacc(ret);
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
                    return new Hacc(std::move(a));
                }
                default: a.push_back(parse_term()); break;
            }
        }
    }
    std::vector<Hacc*> parse_arglist () {
        std::vector<Hacc*> args;
        p++;  // for the (
        for (;;) {
            parse_ws();
            switch (look()) {
                case EOF: throw error("Macro argument list not terminated");
                case ':': throw error("Cannot have : in an argument list");
                case ',': p++; break;
                case ')': p++; {
                    return args;
                }
                default: args.push_back(parse_term()); break;
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
                    return new Hacc(std::move(o));
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
                default: o.push_back(Pair(key, parse_term())); break;
            }
        }
    }
    Hacc* parse_parens () {
        p++;  // for the (
        Hacc* r = parse_term();
        parse_ws();
        if (look() == ')') p++;
        else {
            throw error("Extra stuff in parens");
        }
        return r;
    }
     // TODO: separate vars and assignments
    Hacc* parse_var () {
        p++;  // for the $
        String name = parse_ident("An variable name after $");
        parse_ws();
        if (look() == '=') {
            p++;
            Hacc* value = parse_term();
            return new Hacc(Assignment{new Hacc(Var{name}), value});
        }
        else return new Hacc(Var{name});
    }
    Hacc* parse_address () {
        p++;  // for the &
        if (look() == '$')
            return new Hacc(Address{parse_var()});
        else throw error("Can only take the address of a variable currently.");
    }
    Hacc* parse_bareword () {
         // A previous switch ensures this is a bare word and not a string.
        String word = parse_ident("An ID of some sort (this shouldn't happen)");
        if (word == "null")
            return new Hacc(null);
        else if (word == "false")
            return new Hacc(false);
        else if (word == "true")
            return new Hacc(true);
        else if (word == "nan" || word == "inf") {
            p -= 3;
            return parse_floating();
        }
        else if (look() == '(') {
            return new Hacc(Macro{word, parse_arglist()});
        }
        else {
            return new Hacc(word);
        }
    }
    Hacc* parse_chain (Hacc* subject) {
        if (look() == '.') {
            p++;
            if (isdigit(look())) {
                size_t index;
                uint len;
                if (!sscanf(safebuf(), "%lu%n", &index, &len))
                    throw error("Weird number in element dereference");
                p += len;
                return parse_chain(new Hacc(Elem{subject, index}));
            }
            else if (isalpha(look()) || look() == '_' || look() == '"') {
                String name = parse_ident("attribute name");
                return parse_chain(new Hacc(Attr{subject, name}));
            }
            else throw error("dot not followed by an ident or number");
        }
        else return subject;
    }

    Hacc* parse_constraint () {
        p++;  // for the #
        return new Hacc(Constraint{parse_ident("type after '#'"), parse_term()});
    }

    Hacc* parse_term () {
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
            case '9': return parse_chain(parse_numeric());
            case '#': return parse_constraint();
            case '~': return parse_chain(parse_bitrep());
            case '"': return parse_chain(parse_string());
            case '[': return parse_chain(parse_array());
            case '{': return parse_chain(parse_object());
            case '(': return parse_chain(parse_parens());
            case '$': return parse_var();
            case '&': return parse_chain(parse_address());
            case '_': return parse_chain(parse_bareword());
            case '<': return parse_chain(parse_heredoc());
            default:
                if (isalnum(next))
                    return parse_chain(parse_bareword());
                else throw error("Unrecognized character " + String(1, next));
        }
    }
    Hacc* parse_all () {
        Hacc* r = parse_term();
        parse_ws();
        if (look() == EOF) return r;
        else {
            delete r;
            throw error("Extra stuff at end of document");
        }
    }
    Hacc* parse () {
        try { return parse_all(); }
        catch (Error e) { return new Hacc(Error(e)); }
    }
};

 // Finally:
Hacc* hacc_from_string (const String& s, String filename) { return Parser(s, filename).parse(); }
Hacc* hacc_from_string (const char* s, String filename) { return Parser(s, filename).parse(); }
Hacc* string_to_hacc (const String& s, String filename) { return hacc_from_string(s, filename); }
Hacc* string_to_hacc (const char* s, String filename) { return hacc_from_string(s, filename); }

}


