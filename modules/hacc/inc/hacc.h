
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
#include <memory>  // For unique_ptr
#include <vector>
#include <string>


namespace hacc {

 // This is the main type for this header.
struct Hacc;

 // All the types of values.
enum Form {
    UNDEFINED,
    NULLFORM,
    BOOL,
    INTEGER,
    FLOAT,
    DOUBLE,
    STRING,
    POINTER,
    ARRAY,
    OBJECT,
    ERROR
};

const char* form_name (Form);

 // These are the C++ versions of all the value types
 // TODO: allow the includer to redefine these
typedef std::nullptr_t Null;
constexpr Null null = nullptr;
typedef bool Bool;
typedef int64 Integer;
typedef float Float;
typedef double Double;
typedef std::string String;
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
struct Pointer {
    String type;
    String id;
    const std::type_info* cpptype;
    void* addr;
    bool operator== (Pointer o) const { return (cpptype && o.cpptype && *cpptype == *o.cpptype && addr == o.addr) || (type == o.type && id == o.id); }
    template <class C>
    Pointer (String type, String id, C* addr) :
        type(type), id(id), cpptype(&typeid(C)), addr(addr)
    { }
    Pointer (String type, String id, void* addr = null) :
        type(type), id(id), cpptype(null), addr(addr)
    { }
    template <class C>
    Pointer (C* addr) : type(""), id(""), cpptype(&typeid(C)), addr(addr) { }
    template <class C>
    C* get_ptr () const {
       if (!cpptype)
           throw Error("This Pointer Hacc is not associated with a memory address.");
       if (*cpptype != typeid(C))
           throw Error("This Pointer Hacc is not of type <mangled: " + String(typeid(C).name()) + ">.");
       return (C*)addr;
    }
};
template <class T> using VArray = std::vector<T>;
typedef VArray<std::unique_ptr<Hacc>> Array;
template <class T> using Pair = std::pair<String, T>;
template <class T> using Map = std::vector<Pair<T>>;
typedef Map<std::unique_ptr<Hacc>> Object;

enum Flags {
    ADVERTISE_ID = 1,
    ADVERTISE_TYPE = 2
};


struct Value {
    Form form;
    union {
        Null n;
        Bool b;
        Integer i;
        Float f;
        Double d;
        String s;
        Pointer p;
        Array a;
        Object o;
        Error e;
    };
    ~Value ();
    Value (Value&& rv);
    void set (Value&& rv);
    Value (Null n) : form(NULLFORM), n(n) { }
    Value (Bool b) : form(BOOL), b(b) { }
    Value (char i) : form(INTEGER), i(i) { }
    Value (int8 i) : form(INTEGER), i(i) { }
    Value (uint8 i) : form(INTEGER), i(i) { }
    Value (int16 i) : form(INTEGER), i(i) { }
    Value (uint16 i) : form(INTEGER), i(i) { }
    Value (int32 i) : form(INTEGER), i(i) { }
    Value (uint32 i) : form(INTEGER), i(i) { }
    Value (int64 i) : form(INTEGER), i(i) { }
    Value (uint64 i) : form(INTEGER), i(i) { }
    Value (Float f) : form(FLOAT), f(f) { }
    Value (Double d) : form(DOUBLE), d(d) { }
    Value (const String& s) : form(STRING), s(s) { }
    Value (String&& s) : form(STRING), s(std::forward<String>(s)) { }
    Value (const Pointer& p) : form(POINTER), p(p) { }
    Value (Pointer&& p) : form(POINTER), p(std::forward<Pointer>(p)) { }
    //Value (const Array& a) : form(ARRAY), a(a) { }
    Value (Array&& a) : form(ARRAY), a(std::forward<Array>(a)) { }
    //Value (const Object& o) : form(OBJECT), o(o) { }
    Value (Object&& o) : form(OBJECT), o(std::forward<Object>(o)) { }
    Value (Error e) : form(ERROR), e(e) { }
    Value () : form(UNDEFINED) { }
};

struct Hacc {
    Value value;
    String type;
    String id;
    uint32 flags = 0;
    Hacc (Value&& value, String type = "", String id = "", uint32 flags = 0) :
        value(std::forward<Value>(value)), type(type), id(id), flags(flags)
    { }
    Form form () { return value.form; }
    Null     get_null () const;
    Bool     get_bool () const;
    Integer  get_integer () const;
    Float    get_float () const;
    Double   get_double () const;
    const String&  get_string () const;
    const Pointer& get_pointer () const;
    const Array&   get_array () const;
    const Object&  get_object () const;
    Hacc& get_elem (uint) const;
    bool has_attr (String) const;
    Hacc& get_attr (String) const;
    void add_elem (Hacc&&);
    void add_attr (String, Hacc&&);

    Error form_error (String) const;
    String error_message () const;
    bool defined () const { return value.form != UNDEFINED; }
};


}

#endif
