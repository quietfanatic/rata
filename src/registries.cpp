
 // This is for classes that automatically add themselves to containers

 // You need to specify a name when constructing a Named

#ifdef HEADER

 // Store a pointer to the only one of this type
 // Access it with Unique<Thing>::it or the<Thing>()
template <class C>
struct Unique {
    static C* it;
    void activate () {
        if (it)
            WARN("Warning: Duplicate instantiation of Unique class.\n");
        it = static_cast<C*>(this); }
    void deactivate () {
        if (it != static_cast<C*>(this))
            WARN("Warning: Tried to deactivate non-active member of Unique class.\n");
        it = NULL;
    }
    Unique () { activate(); }
    ~Unique () { deactivate(); }
    Unique (bool active) { if (active) activate(); }
};
template <class C> C* Unique<C>::it = NULL;
 // the<Game_Settings>()
template <class C>
inline C* the () { return Unique<C>::it; }

 // Store all of this class in an intrusive doubly-linked list
template <class C>
struct Listed {
    C* next;
    C* prev;
    static C* first;
    static C* last;
    void activate () { 
       prev = last;
       next = NULL;
       prev->next = static_cast<C*>(this);

       last = static_cast<C*>(this);
       if (!first) first = static_cast<C*>(this);
    }
    void deactivate () {
        if (next) next->prev = prev;
        else if (last == static_cast<C*>(this)) last = prev;
        if (prev) prev->next = next;
        else if (first == static_cast<C*>(this)) first = next;
    }
    Listed () { activate(); }
    ~Listed () { deactivate(); }
    Listed (bool active) { if (active) activate(); }
};
template <class C> C* Listed<C>::first = NULL;
template <class C> C* Listed<C>::last = NULL;

#define FOR_IN_LISTED(i, C) for (auto i = Listed<C>::first; i; i = i->next)


 // Keep a hash of every member of this class.
template <class C>
struct Named {
    static Hash<C*> table;
    CStr name;
    void activate () { if (name) table.insert(name, static_cast<C*>(this)); }
    void deactivate () { if (name) table.remove(name); }
    void set_name (CStr newname) { deactivate(); name = newname; activate(); }
    Named (CStr name = NULL) :name(name) { activate(); }
    ~Named () { deactivate(); }

    static C* lookup (CStr n) { return table.lookup(n); }
};
template <class C> Hash<C*> Named<C>::table;


#define FOR_IN_NAMED(i, C) FOR_IN_HASH(i, Named<C>::table)



#endif
