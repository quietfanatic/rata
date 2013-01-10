#ifndef HAVE_SYSTEM_STATE_H
#define HAVE_SYSTEM_STATE_H

struct Stateful;

struct Game_State {
    Links<Stateful> all_the_things;
};

extern Game_State* current_state;


struct Stateful : Linkable<Stateful> {
    Stateful (Game_State* s = current_state) { if (s) link(s->all_the_things); }
    virtual ~Stateful (); // Gotta be polymorphic.
};

struct Stateful_Type_info {
    const std::type_info& cpptype;
    Stateful* (* allocate )();
};

extern INIT_SAFE(Hash<Stateful_Type_Info>, stateful_types);

template <class C>
Stateful* allocate_to_stateful () { return new C; }

template <class C>
struct Stateful_Type {
    Stateful_Type (Str name) { stateful_types.emplace(name, Stateful_Type_Info{typeid(C), allocate_to_stateful<C>}); }
};


HCB_BEGIN(Game_State) {
    static virtual void describe (hacc::Haccer& h, Game_State& it) {
        using namespace hacc;
        if (Haccer::Validator v = h.validator()) {
            auto a = v->hacc.get_array();
            for (auto iter = a.begin(); iter != a.end(); iter++) {
                elem
            }
        }
    }
} HCB_END(Game_State)




#endif
