
#ifdef HEADER

struct Actor_Array {
	<%= actor_info %>
	bool active;
	bool reactive;
	bool visible;
	VArray<Actor*> children = (0, NULL);
	void serialize (Serializer* s) {
		s->ser(active);
		s->ser(reactive);
		s->ser(visible);
		uint newn = children.n;
		s->ser(newn);
		children.reallocate(newn);  // noop if same size
		s->depth++;
		for (uint i=0; i < children; i++) {
			s->nl();
			if (s->writing()) {
				uint id;
				s->ser(id);
				children[i] = make_actor[id]();
			}
			else {
				s->ser(children[i]->id());
			}
			children[i]->serialize();
		}
		s->depth--;
	}

	void act () {
		if (!active) return;
		for (uint i=0; i < children; i++)
			children[i]->act();
	}
	void react () {
		if (!reactive) return;
		for (uint i=0; i < children; i++)
			children[i]->react();
	}
	void draw () {
		if (!visible) return;
		for (uint i=0; i < children; i++)
			children[i]->draw();
	}
};

#else

#endif



