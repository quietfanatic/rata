
#ifdef HEADER

/*
    Humans And Computers Cooperating
    
    This is a data language intended to be read-writable by both humans and
    computers.  It's designed to allow a program to dump its entire state to
    a file, let that file be saved somewhere or edited by a hacker, and then
    read the state in again and resume as if nothing had happened.
    
    HACC is mostly a superset of JSON.  There are some additions:
     - All datatypes may be prefixed with class and/or address.
        - prefix{...} for objects
        - prefix[...] for arrays
        - prefix(...) for atoms
        - A prefix may look like one of:
            Class_name
            Class_name@address
            @address
        - A Class_name can contain C++ namespace separators and C++ template
           parameter lists, for mapping onto a C++ namespace.  It may also contain
           the * character.  It may not contain parentheses or square brackets
           unless those are in a template parameter list.  Template parameter lists
           are parsed simply, recognizing nested <> pairs but nothing else.
        - An address is an identifier containing alphanumerics and _.  If it's
           a hexadecimal number, it may be a valid memory address.
        - Class names are usually optional, but sometimes they may be necessary.
     - A new datatype is a reference.  It looks like one of:
        &Class_name@address
        &@address
        nil
        - A reference is pretty much just a pointer.  The address may be the memory
           location where the pointed object resided in the originating process.
        - If a document is produced intending to be read by another process, all
           referenced addresses must exist elsewhere in the document (except @0,
           or nil which is an alias for it).  The reading process will update all
           the pointers to suit its own memory layout.
        - No space is allowed after the & or around the @.
     - Floating point literals may be suffixed or replaced with a precise bit
        representation, so they look like one of:
         1.0
         1.0~3f800000
         ~3f800000
        - The bit representation is a hexadecimal number that must be 8 digits long
           for a 32-bit float and 16 digits long for a 64-bit double.  Floats and
           doubles might be interconverted when reading a document, but when writing
           one, its bit representation must match the type used to write it.
        - Programs should always provide a bit representation, but humans don't have
           to.  If a bit representation is present, a program reading it will ignore
           the informal decimal representation.  So if a human changes the value,
           they must erase the bit representation for the program to notice.
        - No space is allowed around the ~.
     - Numbers support 0xdeadbeef hexadecimal format and possibly 0377 octal format.
     - Strings used as keys in an object don't have to be quoted if they're simple.
     - Commas are not required between elements of arrays and objects.  No really!
        But a program should put them in anyway.
     - Any character except " and \ may be unescaped in a string.
   There is one feature JSON has that HACC doesn't:
     - The \uxxxx construct is not allowed in strings.  To put unicode in a string,
        just put it in the string.  Control characters and even NUL are allowed to
        be in a string.  There simply must be no unescaped backslashes and quotes.


*/

namespace hacc {

enum TYPE {
    BOOL,
    NIL,
    INTEGER,
    FLOAT,
    STRING,
    REF,
    OBJECT,
    ARRAY,
};


 // Freeing a Hacc tree will free all subtrees.
struct Hacc {
    TYPE type;
    std::string cl;
    std::string addr;

    Hacc (TYPE type, std::string cl, std::string addr) : type(type), cl(cl), addr(addr) { }
    virtual ~Hacc () { }
    virtual std::string to_string () = 0;

    std::string add_prefix (std::string s) {
        std::string p = cl;
        if (!addr.empty()) p += "@" + addr;
        return p + s;
    }
};

 // Numbers, strings, refs
struct Atom : Hacc {
    Atom (TYPE type, std::string cl, std::string addr) : Hacc(type, cl, addr) { }
    std::string add_prefix (std::string s) {
        if (!cl.empty() || !addr.empty())
            return Hacc::add_prefix("(" + s + ")");
        else return s;
    }
};
 // Arrays and structs
struct Composite : Hacc {
    Composite (TYPE type, std::string cl, std::string addr) : Hacc(type, cl, addr) { }
};

struct Bool : Atom {
    bool val;

    Bool (int64 val, std::string cl = "", std::string addr = "") : Atom(BOOL, cl, addr), val(val) { }
     // No destructor
    std::string to_string () {
        return add_prefix(val ? "true" : "false");
    }
};
struct Nil : Atom {
    Nil (std::string cl = "", std::string addr = "") : Atom(NIL, cl, addr) { }
    std::string to_string () {
        return add_prefix("nil");
    }
};
struct Integer : Atom {
    int64 val;

    Integer (int64 val, std::string cl = "", std::string addr = "") : Atom(INTEGER, cl, addr), val(val) { }
     // No destructor
    std::string to_string () {
        char s [32];
        sprintf(s, "%" PRIi64, val);
        return add_prefix(s);
    }
};
struct Float : Atom {
    float val;

    Float (float val, std::string cl = "", std::string addr = "") : Atom(FLOAT, cl, addr), val(val) { }
     // No destructor
    std::string to_string () {
        char s [32];
        sprintf(s, "%g~%08" PRIx32, val, *(uint32*)&val);
        return add_prefix(s);
    }
};
struct String : Atom {
    std::string val;

    String (std::string val, std::string cl = "", std::string addr = "") : Atom(STRING, cl, addr), val(val) { }
     // No destructor
    std::string to_string () {
        std::string r = "\"";
        for (auto p = val.begin(); p != val.end(); p++) {
            switch (*p) {
                case '"': r += "\\\"";
                case '\\': r += "\\\\";
                case '\b': r += "\b";
                case '\f': r += "\f";
                case '\n': r += "\n";
                case '\r': r += "\r";
                case '\t': r += "\t";
                default: r += std::string(1, *p);
            }
        }
        return add_prefix(r + "\"");
    }
};
struct Ref : Atom {
    std::string target;
    std::string ref;

    Ref (std::string target, std::string ref, std::string cl = "", std::string addr = "") : Atom(REF, cl, addr), target(target), ref(ref) { }
     // No destructor
    std::string to_string () {
        return add_prefix(!target.empty() ? "&" + target + "@" + ref : "&" + ref);
    }
};
 // key: value pair
struct Attr {
    std::string key;
    Hacc* value;

    Attr (std::string key, Hacc* value) : key(key), value(value) { }
//    ~Attr () { delete value; }
};
void destroy_Attrs (Link<Attr>* attrs) {
    if (attrs) {
        delete attrs->head.value;
        destroy_Attrs(attrs->tail);
        delete attrs;
    }
};
struct Object : Composite {
    VArray<Attr> attrs;

    Object (VArray<Attr> attrs, std::string cl = "", std::string addr = "") : Composite(OBJECT, cl, addr), attrs(attrs) { }
    ~Object () { destroy_VArray(attrs); }
    std::string to_string () {
        std::string r = "{";
        for (uint i = 0; i < attrs; i++) {
            r += attrs[i].key + ": " + attrs[i].value->to_string();
            if (i < attrs - 1) r += ", ";
        }
        return add_prefix(r + "}");
    }
};
struct Array : Composite {
    VArray<Hacc*> elems;

    Array (VArray<Hacc*> elems, std::string cl = "", std::string addr = "") : Composite(ARRAY, cl, addr), elems(elems) { }
    ~Array () { destroy_VArray_ptrs(elems); }
    std::string to_string () {
        std::string r = "[";
        for (uint i = 0; i < elems; i++) {
            r += elems[i]->to_string();
            if (i < elems - 1) r += ", ";
        }
        return add_prefix(r + "]");
    }
};


 // Simple enough we don't need a separate lexer.
struct Parser {
    const char* begin;
    const char* p;
    const char* end;
    std::string mess = "";
    Parser (std::string s) : begin(s.data()), p(s.data()), end(s.data()+s.length()) { }
    Parser (const char* s) : begin(s), p(s), end(s + strlen(s)) { }

    int look () { return p == end ? EOF : *p; }

    void eat_ws () {
        while (isspace(look())) p++;
        if (p[0] == '/' && p+1 != end && p[1] == '/') {
            while (look() != '\n' && look() != EOF) p++;
            eat_ws();
        }
    }
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

    Hacc* error (std::string s) {
        mess = s;
        return NULL;
    }
    void eat_ident () {
        while (isalnum(look()) || look() == '_') p++;
    }
    std::string parse_ident () {
        const char* start = p;
        eat_ident();
        return std::string(start, p - start);
    }
    bool eat_params () {
        uint depth = 0;
        for (;;) switch (look()) {
            case '<': p++; depth++; break;
            case '>': p++; if (--depth == 0) return true; else break;
            case EOF: mess = "Template parameter list not terminated"; return false;
            default: p++;
        }
    }
    std::string parse_class () {
        const char* start = p;
        for (;;) {
            eat_ident();
            switch (look()) {
                case '<': if (!eat_params()) return ""; break;
                case ':':
                    if (p+1 != end && p[1] == ':') {
                        p += 2; break;
                    }  // else fall through
                default: return std::string(start, p - start);
            }
        }
    }

     // Parsing of specific types.
     // Any specific type parser may assume the first character is valid.
    Hacc* parse_numeric () {
        int64 val;
        uint len;
        if (!sscanf(safebuf(), "%" SCNi64 "%n", &val, &len))
            return error("Weird number");
        p += len;
        switch (look()) {
            case '~': return parse_bitrep();
            case '.':
            case 'e':
            case 'E':
            case 'p':
            case 'P': p -= len; return parse_float();
            default: return new Integer(val);
        }
    }
    Hacc* parse_float () {
        float val;
        uint len;
        if (!sscanf(safebuf(), "%f%n", &val, &len))
            return error("Weird number");
        p += len;
        switch (look()) {
            case '~': return parse_bitrep();
            default: return new Float(val);
        }
    }
    Hacc* parse_bitrep () {
        p++;  // for the ~
        uint64 rep;
        uint len;
        if (!sscanf(safebuf(), "%" SCNx64 "%n", &rep, &len))
            return error("No precise bitrep found after ~");
        p += len;
        switch (len) {
            case  8: return new Float(*(float*)&rep);
            case 16: return new Float(*(double*)&rep);
            default: return error("Precise bitrep doesn't have 8 or 16 digits");
        }
    }
    std::string parse_stringly () {
        p++;  // for the "
        std::string r = "";
        bool escaped = false;
        for (;;) {
            if (escaped) {
                switch (look()) {
                    case EOF: mess = "String not terminated"; return "";
                    case '"': r += '"'; break;
                    case '\\': r += "\\"; break;
                    case '/': r += "/"; break;
                    case 'b': r += "\b"; break;
                    case 'f': r += "\f"; break;
                    case 'n': r += "\n"; break;
                    case 'r': r += "\r"; break;
                    case 't': r += "\t"; break;
                    default: mess = "Unrecognized escape \\" + std::string(p, 1); return "";
                }
                escaped = false;
            }
            else {
                switch (look()) {
                    case EOF: mess = "String not terminated"; return "";
                    case '"': p++; return r;
                    case '\\': escaped = true; break;
                    default: r += std::string(p, 1);
                }
            }
            p++;
        }
    }
    Hacc* parse_string () {
        std::string r = parse_stringly();
        if (!mess.empty()) return NULL;
        else return new String(r);
    }
    Hacc* parse_array () {
        Link<Hacc*>* elems = NULL;
        Link<Hacc*>** tailp = &elems;
        p++;  // for the [
        eat_ws();
        for (;;) switch (look()) {
            case EOF: mess = "Array not terminated"; goto fail;
            case ':': mess = "Cannot have : in an array"; goto fail;
            case ',': p++; break;
            case ']': {
                auto elemsa = elems->to_VArray();
                destroy_Links(elems);
                return new Array(elemsa);
            }
            default: {
                if (Hacc* thing = parse_thing())
                    build_tail(tailp, thing);
                else goto fail;
            }
        }
        fail: destroy_Links_ptrs(elems); return NULL;
    }
    Hacc* parse_object () {
        Link<Attr>* attrs = NULL;
        Link<Attr>** tailp = &attrs;
        p++;  // for the left brace
        eat_ws();
        std::string key;
        for (;;) {
            switch (look()) {
                case EOF: mess = "Object not terminated"; goto fail;
                case ':': mess = "No key found before : in object"; goto fail;
                case ',': p++; break;
                case '}': {
                    auto attrsa = attrs->to_VArray();
                    destroy_Links(attrs);
                    return new Object(attrsa);
                }
                case '"': key = parse_stringly(); if (!mess.empty()) goto fail; break;
                default: {
                    if (isalpha(look())) key = parse_ident();
                    else { mess = "Key is not string or identifier"; goto fail; }
                }
            }
            eat_ws();
            if (look() != ':') { mess = "Expected : after key"; goto fail; }
            p++;
            eat_ws();
            switch (look()) {
                case EOF: mess = "Object not terminated"; goto fail;
                case ':': mess = "Extra : in object"; goto fail;
                case ',': mess = "Misplaced comma after : in object"; goto fail;
                case '}': mess = "No value found after : in object"; goto fail;
                default: {
                    if (Hacc* value = parse_thing())
                        build_tail(tailp, Attr(key, value));
                    else goto fail;
                }
            }
        }
        fail: destroy_Attrs(attrs); return NULL;
    }
    Hacc* parse_parens () {
        p++;  // for the (
        eat_ws();
        Hacc* v = parse_thing();
        if (!v) return NULL;
        eat_ws();
        if (look() != ')') return error("Extra stuff in parens");
        p++;
        return v;
    }

    Hacc* add_prefix (Hacc* r, std::string cl, std::string addr) {
        if ((!cl.empty() && !r->cl.empty())
         || (!addr.empty() && !r->addr.empty())) {
            delete r;
            return error("Too many prefixes");
        }
        if (r->cl.empty()) r->cl = cl;
        if (r->addr.empty()) r->addr = addr;
        return r;
    }

    Hacc* parse_thing () {
        eat_ws();
        std::string cl = "";
        std::string addr = "";
        for (;;) switch (look()) {
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
            case '9': return add_prefix(parse_numeric(), cl, addr);
            case '~': return add_prefix(parse_bitrep(), cl, addr);
            case '"': return add_prefix(parse_string(), cl, addr);
            case '[': return add_prefix(parse_array(), cl, addr);
            case '{': return add_prefix(parse_object(), cl, addr);
            case '(': return add_prefix(parse_parens(), cl, addr);
            case '@':
                if (addr.empty()) {
                    p++; addr = parse_ident(); break;
                }
                else return error("Too many @addresses");
            default:
                if (isalpha(look()) || look() == '_') {
                    std::string cl_or_sym = parse_class();
                    if (cl_or_sym == "true")
                        return new Bool(true, cl, addr);
                    else if (cl_or_sym == "false")
                        return new Bool(false, cl, addr);
                    else if (cl_or_sym == "nil")
                        return new Nil(cl, addr);
                    else {
                        if (cl.empty()) {
                            cl = cl_or_sym; break;
                        }
                        else return error("Too many classes");
                    }
                }
        }
    }
    Hacc* parse_all () {
        Hacc* r = parse_thing();
        eat_ws();
        if (look() == EOF)
            return r;
        else {
            delete r;
            return error("Extra stuff at end of document");
        }
    }
    std::string diagnose () {
        uint line = 1;
        const char* nl = begin - 1;
        for (const char* p2 = begin; p2 != p; p2++) {
            if (*p2 == '\n') {
                line++;
                nl = p2;
            }
        }
        uint col = p - nl;
        char* ls = new char [17];  // max length of a uint
        sprintf(ls, "%" PRIu32, line);
        char* cs = new char [17];
        sprintf(cs, "%" PRIu32, col);
        std::string r = mess + " at line " + ls + ", col " + cs;
        delete[] ls;
        delete[] cs;
        return r;
    }
};

}

#else

#ifndef DISABLE_TESTS

Tester hacc_tester ("hacc", [](){
    plan(2);
     // Yes, we are leaking memory, because this is all that'll be run.
    is(hacc::Parser("1").parse_all()->to_string(), "1", "Round-trip 1");
    is(hacc::Parser("~3f800000").parse_all()->to_string(), "1~3f800000", "~3f800000 => 1~3f800000");
});

#endif

#endif

