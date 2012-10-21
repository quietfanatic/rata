
#ifdef HEADER

struct Actor {
	virtual uint32 id () = 0;
	virtual CStr name () = 0;
	virtual void act () = 0;  // Do this before each physics step
	virtual void react () = 0;  // Do this after each physics step
	virtual void draw () = 0;
	virtual void serialize (Serializer* s) = 0;  // input or output
    virtual ~Actor () { }
    Actor () { }
};



#else

void serialize_actorp (Serializer* s, Actor*& a) {
	if (s->writing()) {
		uint id = a->id();
		s->ser(id);
	}
	else {
		uint id;
		s->ser(id);
		a = make_actor[id]();
	}
}

#endif



