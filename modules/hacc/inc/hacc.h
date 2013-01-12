
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
    REF,
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
struct Ref {
    String id;
    void* addr;
    bool operator== (Ref o) const { return addr == o.addr || id == o.id; }
    template <class C>
    Ref (String id, C* addr) : id(id), addr(addr) { }
    Ref (String id) : id(id), addr(null) { }
    template <class C>
    Ref (C* addr) : id(""), addr(addr) { }
};
template <class T> using VArray = std::vector<T>;
typedef VArray<const Hacc*> Array;
template <class T> using Map = std::vector<std::pair<String, T>>;
typedef Map<const Hacc*> Object;

template <class T> using Func = std::function<T>;


struct Hacc {
    hacc::String id;
    virtual Form form () const = 0;
    virtual ~Hacc () { }
    struct Null;
    struct Bool;
    struct Integer;
    struct Float;
    struct Double;
    struct String;
    struct Ref;
    struct Array;
    struct Object;
    struct Error;
    struct Undefined;
    Hacc (hacc::String id) : id(id) { }
    hacc::Error form_error (hacc::String expected) const;
#define HACC_GETTER_DECL(type, name, letter) const type* as_##name () const; const hacc::type& get_##name () const;
#define HACC_GETTER_R_DECL(type, name, letter) const type* as_##name () const; hacc::type get_##name () const;
    HACC_GETTER_R_DECL(Null, null, n)
    HACC_GETTER_R_DECL(Bool, bool, b)
    HACC_GETTER_R_DECL(Integer, integer, i)
    HACC_GETTER_R_DECL(Float, float, f)
    HACC_GETTER_R_DECL(Double, double, d)
    HACC_GETTER_DECL(String, string, s)
    HACC_GETTER_DECL(Ref, ref, r)
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
HACC_VARIANT_S(Ref, REF, Ref, r)
HACC_VARIANT(Array, ARRAY,
    hacc::Array a;
    operator const hacc::Array& () const { return a; }
    Array (const hacc::Array& a, hacc::String id = "") : Hacc(id), a(a) { }
    Array (hacc::Array&& a, hacc::String id = "") : Hacc(id), a(a) { }
    Array (std::initializer_list<const Hacc*> l, hacc::String id = "") : Hacc(id), a(l) { }
    size_t n_elems () const { return a.size(); }
    const Hacc* elem (uint i) const { return a.at(i); }
    ~Array () { for (auto p : a) delete p; }
)
HACC_VARIANT(Object, OBJECT,
    hacc::Object o;
    operator const hacc::Object& () const { return o; }
    Object (const hacc::Object& o, hacc::String id = "") : Hacc(id), o(o) { }
    Object (hacc::Object&& o, hacc::String id = "") : Hacc(id), o(o) { }
    Object (std::initializer_list<std::pair<hacc::String, const Hacc*>> l, hacc::String id = "") : Hacc(id), o(l) { }
    size_t n_attrs () const { return o.size(); }
    hacc::String name_at (uint i) const { return o.at(i).first; }
    const Hacc* value_at (uint i) const { return o.at(i).second; }
    bool has_attr (hacc::String s) const;
    const Hacc* attr (hacc::String s) const;
    ~Object () { for (auto& pair : o) delete pair.second; }
)
HACC_VARIANT_S(Error, ERROR, Error, e)

 // Because ugh
#define HACC_NEW_DECL(type, letter, name) static inline const Hacc* new_hacc (type letter, String id = "") { return new const Hacc::name(letter, id); }
#define HACC_NEW_DECL_COPY(type, letter, name) static inline const Hacc* new_hacc (const type& letter, String id = "") { return new const Hacc::name(letter, id); }
#define HACC_NEW_DECL_MOVE(type, letter, name) static inline const Hacc* new_hacc (type&& letter, String id = "") { return new const Hacc::name(std::forward<type>(letter), id); }
static inline const Hacc* new_hacc () { return new const Hacc::Null(); }
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
HACC_NEW_DECL(Ref, r, Ref)
HACC_NEW_DECL_COPY(Array, a, Array)
HACC_NEW_DECL_MOVE(Array, a, Array)
HACC_NEW_DECL_COPY(Object, o, Object)
HACC_NEW_DECL_MOVE(Object, o, Object)
HACC_NEW_DECL(std::initializer_list<const Hacc*>, l, Array)
static inline const Hacc* new_hacc (std::initializer_list<std::pair<std::basic_string<char>, const Hacc*>> l) { return new const Hacc::Object(l); }

static inline std::pair<String, const Hacc*> hacc_attr (String name, const Hacc* val) {
    return std::pair<String, const Hacc*>(name, val);
}
template <class... Args>
static inline std::pair<String, const Hacc*> new_attr (String name, Args... args) {
    return std::pair<String, const Hacc*>(name, new_hacc(args...));
}

}

 // Here's a utility that's frequently used with hacc.
namespace {
     // Auto-deallocate a pointer if an exception happens.
    struct Bomb {
        const hacc::Func<void ()>* detonate;
        Bomb (const hacc::Func<void ()>& d) :detonate(&d) { }
        ~Bomb () { if (detonate) (*detonate)(); }
        void defuse () { detonate = NULL; }
    };
}

#endif
