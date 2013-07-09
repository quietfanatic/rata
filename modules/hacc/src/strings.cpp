
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <sstream>
#include <iomanip>
#include "../inc/files.h"  // for Path
#include "../inc/strings.h"
#include "paths_internal.h"

namespace hacc {

    String escape_string (String unesc) {
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

    String escape_ident (String unesc) {
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

    String path_to_string (Path p, String filename) {
        if (!p) return "null";
        switch (p->type()) {
            case ROOT: {
                auto& pr = static_cast<const PathRoot&>(*p);
                if (pr.filename == filename) return "$";
                else return "$(\"" + escape_string(pr.filename) + "\")";
            }
            case ATTR: {
                auto& pa = static_cast<const PathAttr&>(*p);
                return path_to_string(pa.left, filename)
                     + "." + escape_ident(pa.name);
            }
            case ELEM: {
                auto& pe = static_cast<const PathElem&>(*p);
                std::ostringstream s;
                s << pe.index;
                return path_to_string(pe.left, filename) + "[" + s.str() + "]";
            }
            default: throw X::Corrupted_Path(p);
        }
    }

    String tree_to_string (Tree t, String filename, uint ind, uint prior_ind) {
        switch (t.form()) {
            case NULLFORM: return "null";
            case BOOL: return bool(t) ? "true" : "false";
            case INTEGER: {
                std::ostringstream s;
                s << int64(t);
                return s.str();
            }
            case FLOAT: {
                float f = t.as<float>();
                std::ostringstream s;
                s << f << "~" << std::setfill('0') << std::setw(8) << std::hex << *(uint32*)&f;
                return s.str();
            }
            case DOUBLE: {
                double d = t.as<double>();
                std::ostringstream s;
                s << d << "~" << std::setfill('0') << std::setw(16) << std::hex << *(uint64*)&d;
                return s.str();
            }
            case STRING: return "\"" + escape_string(t.as<String>()) + "\"";
            case ARRAY: {
                const Array& a = t.as<const Array&>();
                if (a.size() == 0) return "[]";
                String r = "[";
                if (ind && a.size() > 1)
                    r += "\n" + indent(prior_ind + 1);
                for (auto& e : a) {
                    if (ind)
                        r += tree_to_string(
                            e, filename,
                            ind - 1, prior_ind + (a.size() > 1)
                        );
                    else
                        r += tree_to_string(e, filename);
                    if (&e != &a.back()) {
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
                const Object& o = t.as<const Object&>();
                if (o.size() == 0) return "{}";
                String r = "{";
                if (ind && o.size() > 1)
                    r += "\n" + indent(prior_ind + 1);
                else
                    r += " ";
                auto nexti = o.begin();
                for (auto i = nexti; i != o.end(); i = nexti) {
                    r += escape_ident(i->first);
                    r += ":";
                    if (ind)
                        r += tree_to_string(
                            i->second, filename,
                            ind - 1, prior_ind + (o.size() > 1)
                        );
                    else
                        r += tree_to_string(i->second, filename, 0, 0);
                    nexti++;
                    if (nexti != o.end()) {
                        if (ind) r += "\n" + indent(prior_ind + 1);
                        else r += " ";
                    }
                }
                if (ind && o.size() > 1)
                    r += "\n" + indent(prior_ind);
                else
                    r += " ";
                return r + "}";
            }
            case PATH: {
                return path_to_string(t.as<Path>(), filename);
            }
            default: throw X::Corrupted_Tree(t);
        }
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

        X::Parse_Error error (String s) {
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
            return X::Parse_Error(s, file, line, col);
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
        Tree parse_numeric () {
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
                default: return Tree(val);
            }
        }
        Tree parse_floating () {
            double val;
            uint len;
            if (!sscanf(safebuf(), "%lg%n", &val, &len))
                throw error("Weird number");
            p += len;
            switch (look()) {
                case '~': return parse_bitrep();
                default: return Tree(val);
            }
        }
        Tree parse_bitrep () {
            p++;  // for the ~
            uint64 rep;
            uint len;
            if (!sscanf(safebuf(), "%" SCNx64 "%n", &rep, &len))
                throw error("Missing precise bitrep after ~");
            p += len;
            switch (len) {
                case  8: return Tree(*(float*)&rep);
                case 16: return Tree(*(double*)&rep);
                default: throw error("Precise bitrep doesn't have 8 or 16 digits");
            }
        }
        Tree parse_string () {
            return Tree(parse_stringly());
        }
        Tree parse_heredoc () {
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
                if (p + terminator.size() > end) throw error("Ran into end of document before " + terminator);
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
                    return Tree(ret);
                }
                while (look() != '\n') {
                    got += look(); p++;
                }
                got += look(); p++;
            }
        }
        Tree parse_array () {
            Array a;
            p++;  // for the [
            for (;;) {
                parse_ws();
                switch (look()) {
                    case EOF: throw error("Array not terminated");
                    case ':': throw error("Cannot have : in an array");
                    case ',': p++; break;
                    case ']': p++; return Tree(std::move(a));
                    default: a.push_back(parse_term()); break;
                }
            }
        }
        Tree parse_object () {
            Object o;
            p++;  // for the {
            String key;
            for (;;) {
                parse_ws();
                switch (look()) {
                    case EOF: throw error("Object not terminated");
                    case ':': throw error("Missing name before : in object");
                    case ',': p++; continue;
                    case '}': p++; return Tree(std::move(o));
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
                    default: o.emplace_back(key, parse_term()); break;
                }
            }
        }
        Tree parse_parens () {
            p++;  // for the (
            Tree r = parse_term();
            parse_ws();
            if (look() == ')') p++;
            else {
                throw error("Extra stuff in parens");
            }
            return r;
        }
        Tree parse_bareword () {
             // A previous switch ensures this is a bare word and not a string.
            String word = parse_ident("An ID of some sort (this shouldn't happen)");
            if (word == "null") return Tree(null);
            else if (word == "false") return Tree(false);
            else if (word == "true") return Tree(true);
            else if (word == "nan" || word == "inf") {
                p -= 3;
                return parse_floating();
            }
            else return Tree(word);
        }
        Path continue_path (Path left) {
            switch (look()) {
                case '.': {
                    p++;
                    String key = parse_ident("After the . in a path");
                    return continue_path(Path(left, key));
                }
                case '[': {
                    p++;
                    switch(look()) {
                        case '0': case '1': case '2': case '3': case '4':
                        case '5': case '6': case '7': case '8': case '9': {
                            auto pp = p;
                            while (isdigit(look())) p++;
                            size_t i;
                            std::istringstream(String(pp, p - pp)) >> i;
                            if (look() != ']')
                                throw error("Expected ] after index, but got " + String(1, look()));
                            p++;
                            return continue_path(Path(left, i));
                        }
                        default: {
                            String k = parse_ident("In the [] in a path");
                            if (look() != ']')
                                throw error("Expected ] after key, but got " + String(1, look()));
                            p++;
                            return continue_path(Path(left, k));
                        }
                    }
                }
                default: return left;
            }
        }
        Tree parse_path () {
            p++;  // for the $
            if (look() == '(') {
                p++;
                String f = parse_ident("After the $( in a path");
                if (look() != ')')
                    throw error("Expected ) after filename, but got " + String(1, look()));
                p++;
                return Tree(continue_path(Path(f)));
            }
            else {
                if (!file.empty())
                    return Tree(continue_path(Path(file)));
                else
                    throw error("Path must contain a filename because the current filename is unknown");
            }
        }

        Tree parse_term () {
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
                case '9': return parse_numeric();
                case '~': return parse_bitrep();
                case '"': return parse_string();
                case '[': return parse_array();
                case '{': return parse_object();
                case '$': return parse_path();
                case '(': return parse_parens();
                case '_': return parse_bareword();
                case '<': return parse_heredoc();
                default:
                    if (isalnum(next))
                        return parse_bareword();
                    else throw error("Unrecognized character " + String(1, next));
            }
        }
        Tree parse_all () {
            Tree r = parse_term();
            parse_ws();
            if (look() == EOF) return r;
            else throw error("Extra stuff at end of document");
        }
        Tree parse () {
            return parse_all();
        }
    };

     // Finally:
    Tree tree_from_string (String s, String filename) { return Parser(s, filename).parse(); }
    Tree tree_from_string (const char* s, String filename) { return Parser(s, filename).parse(); }

     // Forget C++ IO and its crummy diagnostics
    void with_file (String filename, const char* mode, const Func<void (FILE*)>& func) {
        FILE* f = fopen(filename.c_str(), mode);
        if (!f) {
            throw X::Open_Error(filename, errno);
        }
        func(f);
        if (fclose(f) != 0) {
            throw X::Close_Error(filename, errno);
        }
    }

    void tree_to_file (Tree tree, String filename) {
        with_file(filename, "wb", [&](FILE* f){
            String s = tree_to_string(tree, filename);
            fwrite(s.data(), 1, s.size(), f);
            if (s[s.size()-1] != '\n')
                fputc('\n', f);
        });
    }

    Tree tree_from_file (String filename) {
        String r;
        with_file(filename, "rb", [&](FILE* f){
            fseek(f, 0, SEEK_END);
            size_t size = ftell(f);
            rewind(f);
            char* cs = (char*)malloc(size);
            fread(cs, 1, size, f);
            r = String(cs, size);
            free(cs);
        });
        return tree_from_string(r, filename);
    }

    namespace X {
        Open_Error::Open_Error (String filename, int no) :
            IO_Error(
                "Couldn't open file \"" + filename + "\": " + strerror(no),
                filename, no
            )
        { }
        Close_Error::Close_Error (String filename, int no) :
            IO_Error(
                "Couldn't open file \"" + filename + "\": " + strerror(no),
                filename, no
            )
        { }
    }

}


