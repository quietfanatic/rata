#ifndef HAVE_HACC_HACC_H
#define HAVE_HACC_HACC_H

#include <stdint.h>
#include <string>
#include <vector>
#include <functional>
#include <gc/gc_cpp.h>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

namespace hacc {
    typedef decltype(nullptr) Null;
    constexpr Null null = nullptr;

    template <class F> using Func = std::function<F>;
    typedef Func<void (void*)> Cont;

    struct Tree;

    typedef std::string String;
    typedef std::vector<Tree*> Array;
    typedef std::pair<std::string, Tree*> Pair;
    typedef std::vector<Pair> Object;

    namespace X {
        struct Error : std::exception, gc {
            String mess;
            String filename;
            uint line;
            uint col;
            mutable std::string longmess;
            Error (String mess = "", String filename = "", uint line = 0, uint col = 0) :
                mess(mess), filename(filename), line(line), col(col)
            { }

             // std::exception
            const char* what () const noexcept;
        };
         // Things like incorrect attribute names, type mismatches
        struct Logic_Error : Error {
            Logic_Error (String mess) : Error(mess) { }
        };
         // A particularly bad kind of error indicating memory corruption
        struct Corrupted : Error {
            Corrupted (String mess) : Error(mess) { }
        };
         // Indicates an error in the hacc library
        struct Internal_Error : Error {
            Internal_Error (String mess) : Error(mess) { }
        };
         // If multiple errors need to be thrown at once
        struct Combo_Error : Error {
            std::vector<std::exception_ptr> errs;
            Combo_Error (const std::vector<std::exception_ptr>& errs);
            Combo_Error (std::vector<std::exception_ptr>&& errs);
        };
    }
}

#endif
