
 // This header file gives you the tree representation of a HACC document
 // But not any string or file serialization


#ifndef HAVE_HACC_HACC_H
#define HAVE_HACC_HACC_H

#ifndef HAVE_HACC
#define HAVE_HACC
#endif

#include <stdint.h>
 // Do we really still have to do this?
typedef char char8;
typedef int8_t int8;
typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64;
typedef uint64_t uint64;
#include <memory>  // For shared_ptr
#include <vector>
#include <string>


namespace hacc {

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
constexpr Null null = nullptr;
typedef bool Bool;
typedef int64 Integer;
typedef float Float;
typedef double Double;
typedef std::string String;
struct Ref {
    String type;
    String id;
};
template <class T> using VArray = std::vector<T>;
typedef VArray<Hacc> Array;
template <class T> using Pair = std::pair<String, T>;
template <class T> using Map = std::vector<Pair<T>>;
typedef Map<Hacc> Object;
struct Error : std::exception {
    String mess;
    String file;
    uint line;
    uint col;
    const char* what () const noexcept(true);
    Error (String mess, String file = "", uint line = 0, uint col = 0) :
        mess(mess), file(file), line(line), col(col)
    { }
};

 // Now the definition of the main Hacc type
struct Hacc {
    
     // Constructors for all the C++-version-valtypes
    Hacc (      Null    n, String type = "", String id = "", uint32 flags = 0);
    Hacc (      Bool    b, String type = "", String id = "", uint32 flags = 0);
     // Oh gosh we have to do all the integers
    Hacc (      char    i, String type = "", String id = "", uint32 flags = 0);
    Hacc (      int8    i, String type = "", String id = "", uint32 flags = 0);
    Hacc (      uint8   i, String type = "", String id = "", uint32 flags = 0);
    Hacc (      int16   i, String type = "", String id = "", uint32 flags = 0);
    Hacc (      uint16  i, String type = "", String id = "", uint32 flags = 0);
    Hacc (      int32   i, String type = "", String id = "", uint32 flags = 0);
    Hacc (      uint32  i, String type = "", String id = "", uint32 flags = 0);
    Hacc (      int64   i, String type = "", String id = "", uint32 flags = 0);
    Hacc (      uint64  i, String type = "", String id = "", uint32 flags = 0);
    Hacc (      Float   f, String type = "", String id = "", uint32 flags = 0);
    Hacc (      Double  d, String type = "", String id = "", uint32 flags = 0);
    Hacc (const String& s, String type = "", String id = "", uint32 flags = 0);
    Hacc (const Ref&    r, String type = "", String id = "", uint32 flags = 0);
    Hacc (const Array&  a, String type = "", String id = "", uint32 flags = 0);
    Hacc (const Object& o, String type = "", String id = "", uint32 flags = 0);
    Hacc (const Error&  e = Error("Undefined Hacc"), String type = "", String id = "", uint32 flags = 0);
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
          Array&  get_array   () const;
          Object& get_object  () const;
    const Error&  get_error   () const;

          Hacc*   get_attr    (String) const;
     // The assume_* functions do not do checking or coercions.
     // Only use these when you're switching over the valtype()
          Null    assume_null    () const;
          Bool    assume_bool    () const;
          Integer assume_integer () const;
          Float   assume_float   () const;
          Double  assume_double  () const;
    const String& assume_string  () const;
    const Ref&    assume_ref     () const;
          Array&  assume_array   () const;
          Object& assume_object  () const;
    const Error&  assume_error   () const;
     // Manipulating type and id
    String type () const;
    String id () const;
     // These do update in place.  You shouldn't need them often.
    void set_type (String);
    void set_id (String);
     // This sets each only if it has not already been set.
    void default_type_id (String, String);
     // Flag handling
    uint32& flags ();
     // Error handling
    operator Bool () const;  // is true if not Error.
    String error_message () const;  // Returns "" if not Error.

     // For convenience, 
    Error valtype_error (String expected) const;

     // The following are the innards of a Hacc object

    struct Contents;

    template <class T> struct Value;

     // We're giving up and using std yet again.
    std::shared_ptr<Contents> contents;
};

enum Flags {
    ADVERTISE_ID = 1,
    ADVERTISE_TYPE = 2
};



}

#endif
