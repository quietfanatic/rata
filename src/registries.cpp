
 // This is for classes that automatically add themselves to containers

 // All are entirely automatic except for Named, which requires a name when constructing
 // You can deactivate() and activate() objects to put them in and take them out.
 // If you construct with an extra argument of false, they start out inactive.

#ifdef HEADER

 // Store a pointer to the only one of this type
 // Access it with Unique<Thing>::it() or the<Thing>()
 // This is init-safe
template <class C>
struct Unique {
     // Guarantee precense during init-time
    static C*& it () {
        static C* it = NULL;
        return it;
    }
    void activate () {
        if (it())
            WARN("Warning: Duplicate instantiation of Unique class.\n");
        it() = static_cast<C*>(this); }
    void deactivate () {
        if (it() != static_cast<C*>(this))
            WARN("Warning: Tried to deactivate non-active member of Unique class.\n");
        it() = NULL;
    }
    Unique () { activate(); }
    ~Unique () { deactivate(); }
    Unique (bool active) { if (active) activate(); }
};
 // the<Game_Settings>()
template <class C>
inline C* the () { return Unique<C>::it(); }

 // Store all of this class in an intrusive doubly-linked list
 // This is NOT init-safe (for speed), though it may have to become init-safe in future versions.
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

#define FOR_IN_LISTED(i, C) for (auto i = Listed<C>::first; i; i = i->next)


 // Keep a hash of every member of this class.
 // This is init-safe.
template <class C>
struct Named {
     // Guarantee presence during init time
    static Hash<C*>& table () {
        static Hash<C*> table;
        return table;
    }
    CStr name;
    void activate () { if (name) table().insert(name, static_cast<C*>(this)); }
    void deactivate () { if (name) table().remove(name); }
    void set_name (CStr newname) { deactivate(); name = newname; activate(); }
    Named (CStr name = NULL) :name(name) { activate(); }
    ~Named () { deactivate(); }

    static C* lookup (CStr n) { return table().lookup(n); }
};


#define FOR_IN_NAMED(i, C) FOR_IN_HASH(i, Named<C>::table())



#endif
