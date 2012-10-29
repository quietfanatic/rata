
#ifdef HEADER
#define HAVE_HACC

//  Humans And Computers Cooperating
//  
//  This is a data language intended to be read-writable by both humans and
//  computers.  It's designed to allow a program to dump its entire state to
//  a file, let that file be saved somewhere or edited by a hacker, and then
//  read the state in again and resume as if nothing had happened.
//
//  HACC is basically just JSON with types, references, and precise floating
//  point values.  This spec assumes you know JSON and its terminology.
//
//  Clarification of terms:
//   - An 'ident' is a bareword consisting of ASCII letters or digits, underscores,
//      and hyphens.  An ident may begin with any of these, including a digit or
//      a hyphen.  Any place where you have an ident you may replace it with a
//      quoted string containing anything.  "This" is equivalent to This.
//   - A 'reader' is something that reads a HACC document, either a program or a
//      human.
//   - Likewise, a 'writer' is either a program or a human.
//   - A 'value' is a JSON-like thing, such as a number or an array
//   - A 'type' is a string representing the meaning of a JSON-like value.  This
//      can be any string.
//   - A 'valtype' is the JSON-like type of a value.  Supported valtypes are
//      nulls, booleans, integers, floats, doubles, strings, references, arrays, and
//      objects.
//   - An 'atom' is a null, a boolean, an number, a string, or a reference.
//
//  Features HACC has that JSON doesn't:
//   - All values may be prefixed with a type and/or an id:
//         - prefix{object}
//         - prefix[array]
//         - prefix(atom)
//      - The 'extra' part looks like one of these:
//         - #type
//         - #type@id
//         - @id
//         - @id#type
//      - A type is either an ident or a quoted string.  This clarifies the meaning
//         of the value that follows.  If the reader will know what type the value
//         is supposed to be, the #type is optional.
//      - An id is also an ident or a quoted string.  If the document
//         was written by a program, this might be a memory address.  If it was
//         written by a human, this might be a symbol.  No two values in one
//         document should have the same id unless those values are exactly
//         identical and would occupy the same memory location in a program (in other
//         words, the same object got serialized twice somehow).  This should be
//         rare, and may indicate a bug.
//      - No space is allowed between these tokens.
//   - A new atomic valtype is a reference.
//      - It looks like one of these:
//         - &@id
//         - &#type@id
//         - &@id#type
//      - A reference can represent a pointer in a program, or anything that
//         refers to another object but is not that object itself.
//      - The #type is usually optional, but a writing program may want to add it
//         anyway for the convenience of a reading human.
//      - If a document is written to a file, then every id that is referenced must
//         exist elsewhere in the document.  If it is not written in a file, then
//         references may rely on shared knowledge.  For example, an interactive
//         program might use memory locations as its ids, and allow a human to
//         reference a location that the program informed them of earlier.
//      - The id 0 is not special.  If memory addresses are being used as ids, then
//         the NULL pointer should be displayed as null instead of a reference.
//      - No space is allowed between these tokens.
//   - Hacc differentiates between integers and floats.  The maximum precision of
//      integers is implementation-specific, but must be at least 32 bits signed.
//   - Floating point literals may be suffixed or replaced with a precise bit
//      representation.
//      - So they look like one of these:
//         - 1.0
//         - 1.0~3f800000
//         - ~3f800000
//      - The bit representation is a hexadecimal number that must be 8 digits long
//         for a 32-bit float and 16 digits long for a 64-bit double.  Floats and
//         doubles might be interconverted when reading a document, but when writing
//         one, its bit representation must match the type used to write it.
//      - Programs should always provide a bit representation, but humans don't have
//         to.  If a bit representation is present, a program reading it will ignore
//         the informal decimal representation.  So if a human changes the value,
//         they must erase the bit representation for the program to notice.
//      - No space is allowed around the ~.
//      - If there is no precise bit representation, a floating point literal will be
//         taken to have double precision.
//   - Integers can be converted into floats and doubles, but floats and doubles
//      cannot be converted into integers.  Floats can be converted to doubles and
//      doubles can be converted to floats, but in the interest of maintaining
//      precision, the differentiation between floats and doubles should be
//      maintained as long as possible.
//   - The names of pairs in an object may be either quoted strings or idents.
//   - Any character except " and \ may be unescaped in a string, but it is
//      recommended that writers escape \b, \r, \n, \f, and \t.
//
//  Features JSON has that HACC doesn't:
//   - The \uxxxx construct in strings is not supported.  To put unicode in a string,
//      just put it in the string.  Control characters and even NUL are allowed to
//      be in a string.  There simply must be no unescaped backslashes and quotes.



namespace hacc {

struct Hacc_Contents;  // TODO: make this reference-counted
struct Hacc;

 // All the types of values.
enum Valtype {
    VALNULL,
    BOOL,
    INTEGER,
    FLOAT,
    DOUBLE,
    STRING,
    REF,
    ARRAY,
    OBJECT,
    ERROR
};
const char* valtype_name (Valtype);

 // TODO: allow the includer to redefine these
typedef std::nullptr_t Null;
const auto null = nullptr;
typedef bool Bool;
typedef int64 Integer;
typedef float Float;
typedef double Double;
typedef std::string String;
struct Ref {
    String type;
    String id;
};
struct Error : std::exception {
    String mess;
    String file;
    uint line;
    uint col;
    const char* what () const throw() {
        char ls [32]; sprintf((char*)ls, "%" PRIu32, line);
        char cs [32]; sprintf((char*)cs, "%" PRIu32, col);
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
    Error (String mess, String file = "", uint line = 0, uint col = 0) :
        mess(mess), file(file), line(line), col(col)
    { }
};
typedef std::vector<Hacc> Array;
struct Pair;
typedef std::vector<Pair> Object;
struct Hacc {
     // Giving up and using std yet again.
    std::shared_ptr<Hacc_Contents> contents;

    Hacc (      Null    n = null, String type = "", String id = "");
    Hacc (      Bool    b, String type = "", String id = "");
     // Oh gosh we have to do all the integers
    Hacc (      char    i, String type = "", String id = "");
    Hacc (      int8    i, String type = "", String id = "");
    Hacc (      uint8   i, String type = "", String id = "");
    Hacc (      int16   i, String type = "", String id = "");
    Hacc (      uint16  i, String type = "", String id = "");
    Hacc (      int32   i, String type = "", String id = "");
    Hacc (      uint32  i, String type = "", String id = "");
    Hacc (      int64   i, String type = "", String id = "");
    Hacc (      uint64  i, String type = "", String id = "");
    Hacc (      Float   f, String type = "", String id = "");
    Hacc (      Double  d, String type = "", String id = "");
    Hacc (const String& s, String type = "", String id = "");
    Hacc (const Ref&    r, String type = "", String id = "");
    Hacc (const Array&  a, String type = "", String id = "");
    Hacc (const Object& o, String type = "", String id = "");
    Hacc (const Error&  e, String type = "", String id = "");
    String type () const;
    String id () const;
    Valtype valtype () const;

    operator Bool () const;  // returns true if not Error.
    String error_message () const;

          Null    assume_null    () const;
          Bool    assume_bool    () const;
          Integer assume_integer () const;
          Float   assume_float   () const;
          Double  assume_double  () const;
    const String& assume_string  () const;
    const Ref&    assume_ref     () const;
    const Array&  assume_array   () const;
    const Object& assume_object  () const;
    const Error&  assume_error   () const;
          Null    get_null    () const;
          Bool    get_bool    () const;
          Integer get_integer () const;
          Float   get_float   () const;
          Double  get_double  () const;
    const String& get_string  () const;
    const Ref&    get_ref     () const;
    const Array&  get_array   () const;
    const Object& get_object  () const;
    const Error&  get_error   () const;

    String to_string () const;
    String value_to_string () const;

    Error valtype_error (String what) const {
        if (valtype() == ERROR) return assume_error();
        else return Error("This Hacc is not " + what + ", it's of type " + valtype_name(valtype()));
    }
};
typedef std::vector<Hacc> Array;
struct Pair {
    String name;
    Hacc value;
};
typedef std::vector<Pair> Object;
 // Here we're mapping types to their valtypes.
template <class VTYPE> Valtype valtype_of () {
    static_assert(sizeof(VTYPE)?false:false, "The aforementioned VTYPE isn't a HACC value type.");
    return VALNULL;
}
template <> Valtype valtype_of<Null   > () { return VALNULL; }
template <> Valtype valtype_of<Bool   > () { return BOOL   ; }
template <> Valtype valtype_of<Integer> () { return INTEGER; }
template <> Valtype valtype_of<Float  > () { return FLOAT  ; }
template <> Valtype valtype_of<Double > () { return DOUBLE ; }
template <> Valtype valtype_of<String > () { return STRING ; }
template <> Valtype valtype_of<Ref    > () { return REF    ; }
template <> Valtype valtype_of<Array  > () { return ARRAY  ; }
template <> Valtype valtype_of<Object > () { return OBJECT ; }
template <> Valtype valtype_of<Error  > () { return ERROR  ; }
const char* valtype_name (Valtype t) {
    switch (t) {
        case VALNULL: return "null";
        case BOOL: return "bool";
        case INTEGER: return "integer";
        case FLOAT: return "float";
        case DOUBLE: return "double";
        case STRING: return "string";
        case REF: return "ref";
        case ARRAY: return "array";
        case OBJECT: return "object";
        case ERROR: return "error";
        default: return "corrupted";
    }
}

 // So, I tried a bunch of ways of implementing this with a union.  Made a big
 //  mess because of reference-counted strings and whatnot.  In the end we're
 //  just resorting to subtyping.
 
struct Hacc_Contents {
    String type;
    String id;
    Valtype valtype;
    Hacc_Contents (String type, String id, Valtype valtype) :
        type(type), id(id), valtype(valtype)
    { }
};

 // Ooh, I forgot we could do things like this.  This isn't so bad after all
 //  In fact why didn't I think of templates before, when I had so much code
 //  repetition going on?
template <class T>
struct Hacc_Value : Hacc_Contents {
    T value;
    Hacc_Value (String type, String id, const T& value) :
        Hacc_Contents(type, id, valtype_of<T>()), value(value)
    { }
};


 // Escape string according to HACC rules
 // Does not add quotes
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
 // unescape is harder to abstract out, so we'll wait till we need it.

 // Escape ident according to HACC rules
 // Does add quotes if necessary
String escape_ident (String unesc) {
    if (unesc.empty()) return "\"\"";
    for (auto p = unesc.begin(); p != unesc.end(); p++) {
        if (!isalnum(*p) && *p != '_' && *p != '-')
            return "\"" + escape_string(unesc) + "\"";
    }
    return unesc;
}


 // Actually implement Hacc's methods
Hacc::Hacc (      Null    n, String type, String id) : contents(new Hacc_Value<Null   >(type, id, n)) { }
Hacc::Hacc (      Bool    b, String type, String id) : contents(new Hacc_Value<Bool   >(type, id, b)) { }
Hacc::Hacc (      char    i, String type, String id) : contents(new Hacc_Value<Integer>(type, id, i)) { }
Hacc::Hacc (      int8    i, String type, String id) : contents(new Hacc_Value<Integer>(type, id, i)) { }
Hacc::Hacc (      uint8   i, String type, String id) : contents(new Hacc_Value<Integer>(type, id, i)) { }
Hacc::Hacc (      int16   i, String type, String id) : contents(new Hacc_Value<Integer>(type, id, i)) { }
Hacc::Hacc (      uint16  i, String type, String id) : contents(new Hacc_Value<Integer>(type, id, i)) { }
Hacc::Hacc (      int32   i, String type, String id) : contents(new Hacc_Value<Integer>(type, id, i)) { }
Hacc::Hacc (      uint32  i, String type, String id) : contents(new Hacc_Value<Integer>(type, id, i)) { }
Hacc::Hacc (      int64   i, String type, String id) : contents(new Hacc_Value<Integer>(type, id, i)) { }
Hacc::Hacc (      uint64  i, String type, String id) : contents(new Hacc_Value<Integer>(type, id, i)) { }
Hacc::Hacc (      Float   f, String type, String id) : contents(new Hacc_Value<Float  >(type, id, f)) { }
Hacc::Hacc (      Double  d, String type, String id) : contents(new Hacc_Value<Double >(type, id, d)) { }
Hacc::Hacc (const String& s, String type, String id) : contents(new Hacc_Value<String >(type, id, s)) { }
Hacc::Hacc (const Ref&    r, String type, String id) : contents(new Hacc_Value<Ref    >(type, id, r)) { }
Hacc::Hacc (const Array&  a, String type, String id) : contents(new Hacc_Value<Array  >(type, id, a)) { }
Hacc::Hacc (const Object& o, String type, String id) : contents(new Hacc_Value<Object >(type, id, o)) { }
Hacc::Hacc (const Error&  e, String type, String id) : contents(new Hacc_Value<Error  >(type, id, e)) { }

String Hacc::type () const { return contents->type; }
String Hacc::id () const { return contents->id; }
Valtype Hacc::valtype () const { return contents->valtype; }
      Null    Hacc::assume_null    () const { return static_cast<Hacc_Value<Null   >&>(*contents).value; }
      Bool    Hacc::assume_bool    () const { return static_cast<Hacc_Value<Bool   >&>(*contents).value; }
      Integer Hacc::assume_integer () const { return static_cast<Hacc_Value<Integer>&>(*contents).value; }
      Float   Hacc::assume_float   () const { return static_cast<Hacc_Value<Float  >&>(*contents).value; }
      Double  Hacc::assume_double  () const { return static_cast<Hacc_Value<Double >&>(*contents).value; }
const String& Hacc::assume_string  () const { return static_cast<Hacc_Value<String >&>(*contents).value; }
const Ref&    Hacc::assume_ref     () const { return static_cast<Hacc_Value<Ref    >&>(*contents).value; }
const Array&  Hacc::assume_array   () const { return static_cast<Hacc_Value<Array  >&>(*contents).value; }
const Object& Hacc::assume_object  () const { return static_cast<Hacc_Value<Object >&>(*contents).value; }
const Error&  Hacc::assume_error   () const { return static_cast<Hacc_Value<Error  >&>(*contents).value; }
      Null    Hacc::get_null    () const { if (valtype() == VALNULL) return assume_null(); else throw valtype_error("a null"); }
      Bool    Hacc::get_bool    () const { if (valtype() == BOOL) return assume_bool(); else throw valtype_error("a bool"); }
      Integer Hacc::get_integer () const { if (valtype() == INTEGER) return assume_integer(); else throw valtype_error("an integer"); }
      Float   Hacc::get_float   () const {
    switch (valtype()) {
         // Automatic conversion
        case INTEGER: return assume_integer();
        case FLOAT:   return assume_float();
        case DOUBLE:  return assume_double();
        default: throw valtype_error("a number");
    }
}
      Double  Hacc::get_double  () const {
    switch (valtype()) {
         // Automatic conversion
        case INTEGER: return assume_integer();
        case FLOAT:   return assume_float();
        case DOUBLE:  return assume_double();
        default: throw valtype_error("a number");
    }
}
const String& Hacc::get_string  () const { if (valtype() == STRING) return assume_string(); else throw valtype_error("a string"); }
const Ref&    Hacc::get_ref     () const { if (valtype() == REF) return assume_ref(); else throw valtype_error("a ref"); }
const Array&  Hacc::get_array   () const { if (valtype() == ARRAY) return assume_array(); else throw valtype_error("an array"); }
const Object& Hacc::get_object  () const { if (valtype() == OBJECT) return assume_object(); else throw valtype_error("an object"); }
 // Phew!  So many lines for such simple concepts.


 // Serialize the value part to a string
String Hacc::value_to_string () const {
    switch (valtype()) {
        case VALNULL: return "null";
        case BOOL: return assume_bool() ? "true" : "false";
        case INTEGER: {
            char r[32];
            sprintf(r, "%" PRIi64, assume_integer());
            return r;
        }
        case FLOAT: {
            char r[32];
            float f = assume_float();
            sprintf(r, "%g~%08" PRIx32, f, *(uint32*)&f);
            return r;
        }
        case DOUBLE: {
            char r [64];
            double d = assume_double();
            sprintf(r, "%lg~%016" PRIx64, assume_double(), *(uint64*)&d);
            return r;
        }
        case STRING: return "\"" + escape_string(assume_string()) + "\"";
        case REF: {
            const Ref& r = assume_ref();
            return (r.type.empty() ? "&" : "&#" + escape_ident(r.type))
                 + "@" + escape_ident(r.id);
        }
        case ARRAY: {
            const Array& a = assume_array();
            String r = "[";
            for (auto i = a.begin(); i != a.end(); i++) {
                r += i->to_string();
                if (i + 1 != a.end()) r += ", ";
            }
            return r + "]";
        }
        case OBJECT: {
            const Object& o = assume_object();
            String r = "{";
            for (auto i = o.begin(); i != o.end(); i++) {
                r += escape_ident(i->name);
                r += ": ";
                r += i->value.to_string();
                if (i + 1 != o.end()) r += ", ";
            }
            return r + "}";
        }
        case ERROR: throw assume_error();
        default: throw Error("Corrupted Hacc tree\n");
    }
}
String Hacc::to_string () const {
    String r = value_to_string();
    if ((!type().empty() || !id().empty())
     && valtype() != ARRAY && valtype() != OBJECT)
        r = "(" + r + ")";
    if (!id().empty()) r = "@" + escape_ident(id()) + r;
    if (!type().empty()) r = "#" + escape_ident(type()) + r;
    return r;
}

Hacc::operator Bool () const { return valtype() != ERROR; }
String Hacc::error_message () const { return *this ? "" : assume_error().what(); }

 // Simple enough we don't need a separate lexer.
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
    String parse_type () {
        p++;  // For the #
        return parse_ident("a type after #");
    }
    String parse_id () {
        p++;  // For the @
        return parse_ident("an id after @");
    }

     // Parsing of specific valtypes.
     // This one could return an int, float, or double.
    Hacc parse_numeric () {
        int64 val;
        uint len;
        if (!sscanf(safebuf(), "%" SCNi64 "%n", &val, &len))
            throw error("Weird number");
        p += len;
        switch (look()) {
            case '~': return parse_bitrep();
            case '.':
            case 'e':
            case 'E':
            case 'p':  // backtrack!
            case 'P': p -= len; return parse_floating();
            default: return Hacc(val);
        }
    }
    Hacc parse_floating () {
        double val;
        uint len;
        if (!sscanf(safebuf(), "%lg%n", &val, &len))
            throw error("Weird number");
        p += len;
        switch (look()) {
            case '~': return parse_bitrep();
            default: return Hacc(val);
        }
    }
    Hacc parse_bitrep () {
        p++;  // for the ~
        uint64 rep;
        uint len;
        if (!sscanf(safebuf(), "%" SCNx64 "%n", &rep, &len))
            throw error("Missing precise bitrep after ~");
        p += len;
        switch (len) {
            case  8: return Hacc(*(float*)&rep);
            case 16: return Hacc(*(double*)&rep);
            default: throw error("Precise bitrep doesn't have 8 or 16 digits");
        }
    }
    Hacc parse_string () {
        return Hacc(parse_stringly());
    }
    Hacc parse_ref () {
        String type;
        String id;
        p++;  // For the &
        if (look() == '#') type = parse_type();
        if (look() == '@') id = parse_id();
        else throw error("Ref is missing an @id");
        return Hacc(Ref{type, id});
    }
    Hacc parse_array () {
        Array a;
        p++;  // for the [
        for (;;) {
            parse_ws();
            switch (look()) {
                case EOF: throw error("Array not terminated");
                case ':': throw error("Cannot have : in an array");
                case ',': p++; break;
                case ']': p++; return Hacc(a);
                default: a.push_back(parse_thing()); break;
            }
        }
    }
    Hacc parse_object () {
        Object o;
        p++;  // for the left brace
        String key;
        for (;;) {
          find_key:
            parse_ws();
            switch (look()) {
                case EOF: throw error("Object not terminated");
                case ':': throw error("Missing name before : in object");
                case ',': p++; goto find_key;
                case '}': p++; return Hacc(o);
                default: key = parse_ident("an attribute name or the end of the object"); break;
            }
          find_separator:
            parse_ws();
            if (look() == ':') p++;
            else throw error("Missing : after name");
          find_value:
            parse_ws();
            switch (look()) {
                case EOF: throw error("Object not terminated");
                case ':': throw error("Extra : in object");
                case ',': throw error("Misplaced comma after : in object");
                case '}': throw error("Missing value after : in object");
                default: o.push_back(Pair{key, parse_thing()}); break;
            }
        }
    }
    Hacc parse_parens () {
        p++;  // for the (
        Hacc r = parse_thing();
        parse_ws();
        if (look() == ')') p++;
        else throw error("Extra stuff in parens");
        return r;
    }

    Hacc add_prefix (Hacc r, String type, String id) {
        if (!type.empty()) {
            if (r.type().empty()) r.contents->type = type;
            else throw error("Too many #types");
        }
        if (!id.empty()) {
            if (r.id().empty()) r.contents->id = id;
            else throw error("Too many @ids");
        }
        return r;
    }

    Hacc parse_thing () {
        parse_ws();
        String type = "";
        String id = "";
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
            case '9': return add_prefix(parse_numeric(), type, id);
            case '~': return add_prefix(parse_bitrep(), type, id);
            case '"': return add_prefix(parse_string(), type, id);
            case '[': return add_prefix(parse_array(), type, id);
            case '{': return add_prefix(parse_object(), type, id);
            case '(': return add_prefix(parse_parens(), type, id);
            case '&': return add_prefix(parse_ref(), type, id);
            case '#':
                if (type.empty()) type = parse_type();
                else throw error("Too many #classes");
                break;
            case '@':
                if (id.empty()) id = parse_id();
                else throw error("Too many @ids");
                break;
            default: {
                if (end - p >= 4 && 0==strncmp(p, "null", 4)) {
                    p += 4;
                    return add_prefix(Hacc(null), type, id);
                }
                if (end - p >= 4 && 0==strncmp(p, "true", 4)) {
                    p += 4;
                    return add_prefix(Hacc(true), type, id);
                }
                if (end - p >= 5 && 0==strncmp(p, "false", 5)) {
                    p += 5;
                    return add_prefix(Hacc(false), type, id);
                }
                else throw error("Unknown sequence");
            }
        }
    }
    Hacc parse_all () {
        Hacc r = parse_thing();
        parse_ws();
        if (look() == EOF) return r;
        else throw error("Extra stuff at end of document");
    }
    Hacc parse () {
        try { return parse_all(); }
        catch (Error e) { return Hacc(e); }
    }
};

 // Finally:
Hacc parse (String s) { return Parser(s).parse(); }
Hacc parse (const char* s) { return Parser(s).parse(); }

}

#else

#ifndef DISABLE_TESTS

void hacc_string_test (hacc::String from, hacc::String to) {
    hacc::Hacc tree = hacc::parse(from);
    const char* name = (hacc::escape_string(from) + " -> " + hacc::escape_string(to)).c_str();
    if (!tree) {
        fail(name);
        printf(" # Parse failed: %s\n", tree.error_message().c_str());
    }
    else is(tree.to_string(), to, name);
}

Tester hacc_tester ("hacc", [](){
    plan(46);
     printf(" # Bools\n");  // 2
    hacc_string_test("true", "true");
    hacc_string_test("false", "false");
     printf(" # Null\n");  // 1
    hacc_string_test("null", "null");
     printf(" # Integers\n");  // 8
    hacc_string_test("1", "1");
    hacc_string_test("5425432", "5425432");
    hacc_string_test("\t 5425432 \n", "5425432");
    hacc_string_test("-532", "-532");
    hacc_string_test("+54", "54");
    hacc_string_test("0x7f", "127");
    hacc_string_test("-0x80", "-128");
    hacc_string_test("+0x100", "256");
     printf(" # Floats\n");  // 6
    hacc_string_test("1~3f800000", "1~3f800000");
    hacc_string_test("1.0~3f800000", "1~3f800000");
    hacc_string_test("1.0", "1~3ff0000000000000");
    hacc_string_test("~3f800000", "1~3f800000");
    hacc_string_test("~3ff0000000000000", "1~3ff0000000000000");
    hacc_string_test("2.0", "2~4000000000000000");
    hacc_string_test("0.5", "0.5~3fe0000000000000");
     printf(" # Strings\n");  // 4
    is(hacc::escape_string("\"\\\b\f\n\r\t"), "\\\"\\\\\\b\\f\\n\\r\\t", "hacc::escape_string does its job");
    hacc_string_test("\"asdfasdf\"", "\"asdfasdf\"");
    hacc_string_test("\"\"", "\"\"");
    hacc_string_test("\"\\\"\\\\\\b\\f\\n\\r\\t\"", "\"\\\"\\\\\\b\\f\\n\\r\\t\"");
     printf(" # Arrays\n");  // 8
    hacc_string_test("[]", "[]");
    hacc_string_test("[1]", "[1]");
    hacc_string_test("[1, 2, 3]", "[1, 2, 3]");
    hacc_string_test("[ 1, 2, 3 ]", "[1, 2, 3]");
    hacc_string_test("[, 1 2,,,, 3,]", "[1, 2, 3]");
    hacc_string_test("[~3f800000, -45, \"asdf]\", null]", "[1~3f800000, -45, \"asdf]\", null]");
    hacc_string_test("[[[][]][[]][][][][[[[[[]]]]]]]", "[[[], []], [[]], [], [], [], [[[[[[]]]]]]]");
    hacc_string_test("[1, 2, [3, 4, 5], 6, 7]", "[1, 2, [3, 4, 5], 6, 7]");
     printf(" # Objects\n");  // 7
    hacc_string_test("{}", "{}");
    hacc_string_test("{\"a\": 1}", "{a: 1}");
    hacc_string_test("{a: 1}", "{a: 1}");
    hacc_string_test("{a: 1, b: 2, ccc: 3}", "{a: 1, b: 2, ccc: 3}");
    hacc_string_test("{ , a: -32 b:\"sadf\" ,,,,,,,c:null,}", "{a: -32, b: \"sadf\", c: null}");
    hacc_string_test("{\"\\\"\\\\\\b\\f\\n\\r\\t\": null}", "{\"\\\"\\\\\\b\\f\\n\\r\\t\": null}");
    hacc_string_test("{a: {b: {c: {} d: {}} e: {}}}", "{a: {b: {c: {}, d: {}}, e: {}}}");
     printf(" # Arrays and Objects\n");  // 2
    hacc_string_test("[{a: 1, b: []} [4, {c: {d: []}}]]", "[{a: 1, b: []}, [4, {c: {d: []}}]]");
    hacc_string_test("{a: []}", "{a: []}");
     printf(" # Refs\n");  // 2
    hacc_string_test("&@an_addr3432", "&@an_addr3432");
    hacc_string_test("&#a_class@an_addr", "&#a_class@an_addr");
     printf(" # Prefixes\n");  // 5
    hacc_string_test("#int32(1)", "#int32(1)");
    hacc_string_test("#bool(false)", "#bool(false)");
    hacc_string_test("#\"VArray<int32>\"[1, 2, 3]", "#\"VArray<int32>\"[1, 2, 3]");
    hacc_string_test("#int32@one(1)", "#int32@one(1)");
    hacc_string_test("#\"int8*\"@cp(&#int8@c)", "#\"int8*\"@cp(&#int8@c)");
});

#endif

#endif

