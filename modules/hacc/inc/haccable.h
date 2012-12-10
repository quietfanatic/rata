
#include "hacc.h"
#include <typeinfo>

namespace hacc {

 // Specialize this to hacc your classes.
template <class C> struct Haccable;

 // This needs to be declared here so that Haccability<> can access it.
struct HaccTable {
    const std::type_info& cpptype_t;
    String type_s;
    void* (* allocate_p ) ();
    void (* deallocate_p ) (void*);
    Hacc (* to_hacc_p ) (const void*);
    // Can't genericize from_hacc because it returns an unboxed thing.
    void (* update_from_hacc_p ) (void*, Hacc);
    void* (* new_from_hacc_p ) (Hacc);
     // TODO
    // String (* generate_id_p ) (void*);
    // void* (* find_by_id_p ) (String);
    // std::vector<Haccribute> attrs;
    // std::vector<Hacclement> elems;

    extern std::unordered_map<const type_info&, HaccTable*> by_cpptype;
    extern std::unordered_map<String, HaccTable*> by_type;

     // Constructor template FTW
    template <class C>
    HaccTable () :
        type_s(""), allocate_p(NULL), deallocate_p(NULL),
        to_hacc_p(NULL, update_from_hacc_p(NULL), new_from_hacc_p(NULL)
    {
        cpptype_t = typeid(C);
        Haccable<C>::describe();
    }
    
};


 // Inherit from this to get the haccable pseudo-DSL
template <class C>
struct Haccabililty {
    static HaccTable table;
     // We're storing from_hacc here because we can't put it in HaccTable.
    static C (* from_hacc_p ) (Hacc);

    static void type (String s) { table.type_s = s; }
    static void allocate (C* (* allocate_p )()) {
        table.allocate_p = (void*(*)())allocate_p;
    }
    static void deallocate (void (* deallocate_p )(C*)) {
        table.deallocate_p = (void(*)(void*))deallocate_p;
    }
    static void to (Hacc (* to_hacc )(const C&)) {
        table.to_hacc_p = (Hacc(*)(const void*))to_hacc_p;
    }
    static void from (C (* from_hacc_p_ )(Hacc)) {
        from_hacc_p = from_hacc_p_;
    }
    static void update_from (void (* update_from_hacc_p )(C&, Hacc)) {
        table.update_from_hacc_p = (void(*)(void*, Hacc))update_from_hacc_p;
    }
    static void new_from (C* (* new_from_hacc_p )(Hacc)) {
        table.new_from_hacc_p = (void*(*)(Hacc))new_from_hacc_p;
    }
}
 // We're kinda assuming these get run in order
template <class C> C (* Haccabililty<C>::from_hacc_p ) (Hacc) = NULL;
template <class C> HaccTable Haccabililty<C>::table = HaccTable::HaccTable<C>();

}




