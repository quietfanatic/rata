#ifndef HAVE_HACC_COMMON_H
#define HAVE_HACC_COMMON_H

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

    struct Unknown { };
    struct Tree;

    typedef std::string String;
    typedef std::vector<Tree> Array;
    typedef std::pair<std::string, Tree> Pair;
    typedef std::vector<Pair> Object;

    struct DPtee {
        virtual ~DPtee () { }  // necessary for easy opacity
        mutable size_t ref_count = 0;
    };
    template <class C>
    struct DPtr {
      private:
        C* p;
        void inc () { if (p) ((DPtee*)p)->ref_count++; }
        void dec () {
            if (p && !--((DPtee*)p)->ref_count)
                delete ((DPtee*)p);
        }
      public:
        DPtr () : p(null) { }
        explicit DPtr (C* p) : p(p) { inc(); }
        DPtr (const DPtr<C>& o) : p(o.p) { inc(); }
        DPtr (DPtr<C>&& o) : p(o.p) { o.p = null; }
        ~DPtr () noexcept { dec(); }
        DPtr& operator = (Null n) { dec(); p = n; }
        DPtr& operator = (const DPtr<C>& o) { dec(); p = o.p; inc(); return *this; }
        DPtr& operator = (DPtr<C>&& o) { dec(); p = o.p; o.p = null; return *this; }
        explicit operator C* () { return p; }
        explicit operator const C* () const { return p; }
        C& operator * () { return *p; }
        const C& operator * () const { return *const_cast<const C*>(p); }
        C* operator -> () { return p; }
        const C* operator -> () const { return const_cast<const C*>(p); }
        operator bool () const { return p; }
    };

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
