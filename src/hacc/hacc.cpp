
namespace hacc {

#ifdef HEADER
#define HAVE_HACC


 // This is the main type for this header.
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
 // Map C++-version valtypes to valtypes
template <class VTYPE> Valtype valtype_of ();
 // Get the string name of the type
const char* valtype_name (Valtype);

 // These are the C++ versions of all the value types
 // TODO: allow the includer to redefine these
typedef std::nullptr_t Null;
 const auto null = nullptr;
typedef bool Bool;
typedef int64 Integer;
typedef float Float;
typedef double Double;
typedef std::string String;
struct Ref;
typedef std::vector<Hacc> Array;
 struct Pair;  // One pair in an object
typedef std::vector<Pair> Object;  // TODO: replace with unordered map
struct Error;

 // Now the definition of the main Hacc type
struct Hacc {
    
     // Constructors for all the C++-version-valtypes
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
     // Get the valtype
    Valtype valtype () const;
     // Extracting values
     // The get_* functions do type checking and number coercion
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
     // The assume_* functions do not do checking or coercions.
     // Only use these when you're switching over the valtype()
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
     // Manipulating type and id
    String type () const;
    String id () const;
     // These do update in place.  You shouldn't need them often.
    void set_type (String);
    void set_id (String);
     // This sets each only if it has not already been set.
    void default_type_id (String, String);
     // Error handling
    operator Bool () const;  // is true if not Error.
    String error_message () const;  // Returns "" if not Error.

     // For convenience, 
    Error valtype_error (String expected) const;

     // The following are the innards of a Hacc object

     // So, I tried a bunch of ways of implementing this with a union.  Made a big
     //  mess because of reference-counted strings and whatnot.  In the end we're
     //  just resorting to subtyping.

    struct Contents {
        String type;
        String id;
        Valtype valtype;
        Contents (String type, String id, Valtype valtype) :
            type(type), id(id), valtype(valtype)
        { }
    };

     // Ooh, I forgot we could do things like this.  This isn't so bad after all
     //  In fact why didn't I think of templates before, when I had so much code
     //  repetition going on?
    template <class T>
    struct Value : Contents {
        T value;
        Value (String type, String id, const T& value) :
            Contents(type, id, valtype_of<T>()), value(value)
        { }
    };

     // We're giving up and using std yet again.
    std::shared_ptr<Contents> contents;
};

 // Now we define the types that depend on Hacc
struct Ref {
    String type;
    String id;
};
struct Pair {
    String name;
    Hacc value;
};

 // This can be the contents of a Hacc or it can be thrown
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

#else // not HEADER


 // map C++ types to their valtypes.
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


 // Actually implement Hacc's methods
Hacc::Hacc (      Null    n, String type, String id) : contents(new Value<Null   >(type, id, n)) { }
Hacc::Hacc (      Bool    b, String type, String id) : contents(new Value<Bool   >(type, id, b)) { }
Hacc::Hacc (      char    i, String type, String id) : contents(new Value<Integer>(type, id, i)) { }
Hacc::Hacc (      int8    i, String type, String id) : contents(new Value<Integer>(type, id, i)) { }
Hacc::Hacc (      uint8   i, String type, String id) : contents(new Value<Integer>(type, id, i)) { }
Hacc::Hacc (      int16   i, String type, String id) : contents(new Value<Integer>(type, id, i)) { }
Hacc::Hacc (      uint16  i, String type, String id) : contents(new Value<Integer>(type, id, i)) { }
Hacc::Hacc (      int32   i, String type, String id) : contents(new Value<Integer>(type, id, i)) { }
Hacc::Hacc (      uint32  i, String type, String id) : contents(new Value<Integer>(type, id, i)) { }
Hacc::Hacc (      int64   i, String type, String id) : contents(new Value<Integer>(type, id, i)) { }
Hacc::Hacc (      uint64  i, String type, String id) : contents(new Value<Integer>(type, id, i)) { }
Hacc::Hacc (      Float   f, String type, String id) : contents(new Value<Float  >(type, id, f)) { }
Hacc::Hacc (      Double  d, String type, String id) : contents(new Value<Double >(type, id, d)) { }
Hacc::Hacc (const String& s, String type, String id) : contents(new Value<String >(type, id, s)) { }
Hacc::Hacc (const Ref&    r, String type, String id) : contents(new Value<Ref    >(type, id, r)) { }
Hacc::Hacc (const Array&  a, String type, String id) : contents(new Value<Array  >(type, id, a)) { }
Hacc::Hacc (const Object& o, String type, String id) : contents(new Value<Object >(type, id, o)) { }
Hacc::Hacc (const Error&  e, String type, String id) : contents(new Value<Error  >(type, id, e)) { }
Valtype Hacc::valtype () const { return contents->valtype; }
      Null    Hacc::assume_null    () const { return static_cast<Value<Null   >&>(*contents).value; }
      Bool    Hacc::assume_bool    () const { return static_cast<Value<Bool   >&>(*contents).value; }
      Integer Hacc::assume_integer () const { return static_cast<Value<Integer>&>(*contents).value; }
      Float   Hacc::assume_float   () const { return static_cast<Value<Float  >&>(*contents).value; }
      Double  Hacc::assume_double  () const { return static_cast<Value<Double >&>(*contents).value; }
const String& Hacc::assume_string  () const { return static_cast<Value<String >&>(*contents).value; }
const Ref&    Hacc::assume_ref     () const { return static_cast<Value<Ref    >&>(*contents).value; }
const Array&  Hacc::assume_array   () const { return static_cast<Value<Array  >&>(*contents).value; }
const Object& Hacc::assume_object  () const { return static_cast<Value<Object >&>(*contents).value; }
const Error&  Hacc::assume_error   () const { return static_cast<Value<Error  >&>(*contents).value; }
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


String Hacc::type () const { return contents->type; }
void Hacc::set_type (String s) { contents->type = s; }
String Hacc::id () const { return contents->id; }
void Hacc::set_id (String s) { contents->id = s; }
void Hacc::default_type_id (String type, String id) {
    if (contents->type.empty()) contents->type = type;
    if (contents->id.empty()) contents->id = id;
}

Hacc::operator Bool () const { return valtype() != ERROR; }
String Hacc::error_message () const { return *this ? "" : assume_error().what(); }

Error Hacc::valtype_error (String expected) const {
    if (valtype() == ERROR) return assume_error();
    else return Error("This Hacc is not " + expected + ", it's of valtype " + valtype_name(valtype()));
}

#endif

}
