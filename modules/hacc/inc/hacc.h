
 // This header file gives you the tree representation of a HACC document
 // But not any string or file serialization


#ifndef HAVE_HACC_HACC_H
#define HAVE_HACC_HACC_H

#include <gc/gc_cpp.h>

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
#include <functional>
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
    VAR,
    POINTER,
    ATTRREF,
    ELEMREF,
    ADDRESS,
    MACROCALL,
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
struct Var {
    String name;
    bool operator== (Var o) const { return name == o.name; }
    Var (String name) : name(name) { }
    Var (const char* name) : name(name) { }
};
struct Pointer {
    const std::type_info* cpptype;
    void* p;
    Pointer () : cpptype(NULL), p(NULL) { }
    Pointer (const std::type_info& cpptype, void* p) : cpptype(&cpptype), p(p) { }
    template <class C> Pointer (C* p) : cpptype(&typeid(C)), p(p) { }
};
struct AttrRef {
    Hacc* subject;
    String name;
    AttrRef (Hacc* s, String n) : subject(s), name(n) { }
};
struct ElemRef {
    Hacc* subject;
    size_t index;
    ElemRef (Hacc* s, size_t i) : subject(s), index(i) { }
};
struct Address {
    Hacc* subject;
    Address (Hacc* s) : subject(s) { }
};
struct MacroCall {
    String name;
    std::vector<Hacc*> args;
    MacroCall (String name, const std::vector<Hacc*>& args) : name(name), args(args) { }
    MacroCall (String name, std::vector<Hacc*>&& args) : name(name), args(args) { }
    MacroCall (String name, std::initializer_list<Hacc*> args) : name(name), args(args) { }
};
template <class T> using VArray = std::vector<T>;
typedef VArray<Hacc*> Array;
template <class T> using Map = std::vector<std::pair<String, T>>;
typedef Map<Hacc*> Object;

template <class T> using Func = std::function<T>;

 // These are garbage collected, so don't delete them.
struct Hacc : gc {
    hacc::String type;
    hacc::String id;
    virtual Form form () const = 0;
    virtual ~Hacc () { }
    struct Null;
    struct Bool;
    struct Integer;
    struct Float;
    struct Double;
    struct String;
    struct Var;
    struct Pointer;
    struct AttrRef;
    struct ElemRef;
    struct Address;
    struct MacroCall;
    struct Array;
    struct Object;
    struct Error;
    struct Undefined;
    Hacc (hacc::String id) : id(id) { }
    hacc::Error form_error (hacc::String expected);
#define HACC_GETTER_DECL(type, name, letter) type* as_##name (); hacc::type& get_##name ();
#define HACC_GETTER_R_DECL(type, name, letter) type* as_##name (); hacc::type get_##name ();
    HACC_GETTER_R_DECL(Null, null, n)
    HACC_GETTER_R_DECL(Bool, bool, b)
    HACC_GETTER_R_DECL(Integer, integer, i)
    HACC_GETTER_R_DECL(Float, float, f)
    HACC_GETTER_R_DECL(Double, double, d)
    HACC_GETTER_DECL(String, string, s)
    HACC_GETTER_DECL(Var, var, v)
    HACC_GETTER_DECL(Pointer, pointer, p)
    HACC_GETTER_DECL(AttrRef, attrref, ar)
    HACC_GETTER_DECL(ElemRef, elemref, er)
    HACC_GETTER_DECL(Address, address, ad)
    HACC_GETTER_DECL(MacroCall, macrocall, mc)
    HACC_GETTER_DECL(Array, array, a)
    HACC_GETTER_DECL(Object, object, o)
    HACC_GETTER_DECL(Error, error, e)
};
#define HACC_VARIANT(name, theform, ...) struct Hacc::name : Hacc { Form form () const { return theform; } __VA_ARGS__ };
#define HACC_VARIANT_S(name, theform, type, letter, ...) \
HACC_VARIANT(name, theform, \
    hacc::type letter; \
    operator hacc::type () { return letter; } \
    name (hacc::type letter, hacc::String id = "") : Hacc(id), letter(letter) { } \
    __VA_ARGS__ \
)
HACC_VARIANT_S(Null, NULLFORM, Null, n, Null (hacc::String id = "") : Hacc(id), n(null) { })
HACC_VARIANT_S(Bool, BOOL, Bool, b)
HACC_VARIANT_S(Integer, INTEGER, Integer, i)
HACC_VARIANT_S(Float, FLOAT, Float, f)
HACC_VARIANT_S(Double, DOUBLE, Double, d)
HACC_VARIANT_S(String, STRING, String, s)
HACC_VARIANT_S(Var, VAR, Var, v)
HACC_VARIANT_S(Address, ADDRESS, Address, ad)
HACC_VARIANT(Pointer, POINTER,
    hacc::Pointer p;
    operator hacc::Pointer& () { return p; }
    Pointer (const hacc::Pointer& p, hacc::String id = "") : Hacc(id), p(p) { }
)
HACC_VARIANT(AttrRef, ATTRREF,
    hacc::AttrRef ar;
    operator hacc::AttrRef& () { return ar; }
    AttrRef (const hacc::AttrRef& ar, hacc::String id = "") : Hacc(id), ar(ar) { }
)
HACC_VARIANT(ElemRef, ELEMREF,
    hacc::ElemRef er;
    operator hacc::ElemRef& () { return er; }
    ElemRef (const hacc::ElemRef& er, hacc::String id = "") : Hacc(id), er(er) { }
)
HACC_VARIANT(MacroCall, MACROCALL,
    hacc::MacroCall mc;
    operator hacc::MacroCall& () { return mc; }
    MacroCall (const hacc::MacroCall& mc, hacc::String id = "") : Hacc(id), mc(mc) { }
)
HACC_VARIANT(Array, ARRAY,
    hacc::Array a;
    operator hacc::Array& () { return a; }
    Array (const hacc::Array& a, hacc::String id = "") : Hacc(id), a(a) { }
    Array (hacc::Array&& a, hacc::String id = "") : Hacc(id), a(a) { }
    Array (std::initializer_list<Hacc*> l, hacc::String id = "") : Hacc(id), a(l) { }
    size_t n_elems () const { return a.size(); }
    Hacc* elem (uint i) const { return a.at(i); }
)
HACC_VARIANT(Object, OBJECT,
    hacc::Object o;
    operator hacc::Object& () { return o; }
    Object (const hacc::Object& o, hacc::String id = "") : Hacc(id), o(o) { }
    Object (hacc::Object&& o, hacc::String id = "") : Hacc(id), o(o) { }
    Object (std::initializer_list<std::pair<hacc::String, Hacc*>> l, hacc::String id = "") : Hacc(id), o(l) { }
    size_t n_attrs () const { return o.size(); }
    hacc::String name_at (uint i) const { return o.at(i).first; }
    Hacc* value_at (uint i) const { return o.at(i).second; }
    bool has_attr (hacc::String s) const;
    Hacc* attr (hacc::String s) const;
)
HACC_VARIANT_S(Error, ERROR, Error, e)

 // Because ugh
#define HACC_NEW_DECL(type, letter, name) static inline Hacc* new_hacc (type letter, String id = "") { return new Hacc::name(letter, id); }
#define HACC_NEW_DECL_COPY(type, letter, name) static inline Hacc* new_hacc (const type& letter, String id = "") { return new Hacc::name(letter, id); }
#define HACC_NEW_DECL_MOVE(type, letter, name) static inline Hacc* new_hacc (type&& letter, String id = "") { return new Hacc::name(std::forward<type>(letter), id); }
static inline Hacc* new_hacc () { return new Hacc::Null(); }
HACC_NEW_DECL(Null, n, Null)
HACC_NEW_DECL(Bool, b, Bool)
HACC_NEW_DECL(char, i, Integer)
HACC_NEW_DECL(int8, i, Integer)
HACC_NEW_DECL(uint8, i, Integer)
HACC_NEW_DECL(int16, i, Integer)
HACC_NEW_DECL(uint16, i, Integer)
HACC_NEW_DECL(int32, i, Integer)
HACC_NEW_DECL(uint32, i, Integer)
HACC_NEW_DECL(int64, i, Integer)
HACC_NEW_DECL(uint64, i, Integer)
HACC_NEW_DECL(Float, f, Float)
HACC_NEW_DECL(Double, d, Double)
HACC_NEW_DECL(String, s, String)
HACC_NEW_DECL(Var, v, Var)
HACC_NEW_DECL(Pointer, p, Pointer)
HACC_NEW_DECL(AttrRef, ar, AttrRef)
HACC_NEW_DECL(ElemRef, er, ElemRef)
HACC_NEW_DECL(Address, ad, Address)
HACC_NEW_DECL_COPY(MacroCall, mc, MacroCall)
HACC_NEW_DECL_MOVE(MacroCall, mc, MacroCall)
HACC_NEW_DECL_COPY(Array, a, Array)
HACC_NEW_DECL_MOVE(Array, a, Array)
HACC_NEW_DECL_COPY(Object, o, Object)
HACC_NEW_DECL_MOVE(Object, o, Object)
HACC_NEW_DECL(std::initializer_list<Hacc*>, l, Array)
static inline Hacc* new_hacc (std::initializer_list<std::pair<std::basic_string<char>, Hacc*>> l) { return new Hacc::Object(l); }

static inline std::pair<String, Hacc*> hacc_attr (String name, Hacc* val) {
    return std::pair<String, Hacc*>(name, val);
}
template <class... Args>
static inline std::pair<String, Hacc*> new_attr (String name, Args... args) {
    return std::pair<String, Hacc*>(name, new_hacc(args...));
}

 // I had a class "Bomb" that acted in place of a "finally" clause.
 // But it was buggy and prone to explode very messily.
 // I'm kicking myself for forgetting about libgc until now.

} // namespace hacc

#endif
