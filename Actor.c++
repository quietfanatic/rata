

#ifdef HEADER

struct Actor : actor::Def {
	int16 id;
	bool active;
	bool awaiting_activation;
	Actor* contents;
	Actor* next;
	Actor* prev;
	Actor* next_active;
	Actor (actor::Def* def);

	virtual void activate ();
	virtual void deactivate ();

	virtual void before_move () { }
	virtual void after_move () { }
	virtual void draw () { }
	
	void take_sorted (Actor* a);
	void take_unsorted (Actor* a);
	virtual void receive (Actor* a);
	virtual void release (Actor* a);
	void produce (Actor* a);

	bool has_body ();
	int get_room ();
};

#else

Actor::Actor (actor::Def* def) :
	actor::Def(*def),
	id( def - actor::saved < actor::n_globals ? def - actor::saved : -1 ),
	active(false),
	contents(NULL),
	next(NULL),
	prev(NULL),
	next_active(NULL)
{ }

void Actor::activate () {
	if (active) {
		printf("Warning: Attempted to activate an active actor.\n");
		return;
	}
	if (awaiting_activation) {
		printf("Warning: Attempted to double-activate an actor.\n");
		return;
	}
	next_active = activation_queue;
	activation_queue = this;
	awaiting_activation = true;
	dbg(2, "Activating 0x%08x\n", this);
}
void Actor::deactivate () {
	if (!active) {
		printf("Warning: Attempted to deactivate an inactive actor.\n");
		return;
	}
	active = false;
	dbg(2, "Deactivating 0x%08x\n", this);
}

void Actor::take_unsorted (Actor* a) {
	if (contents) contents->prev = a;
	a->next = contents;
	contents = a;
}
void Actor::take_sorted (Actor* a) {
	Actor* c;
	if (!contents)
		contents = a;
	else if (a->pos.x > contents->pos.x) {
		a->next = contents;
		contents->prev = a;
		contents = a;
	}
	else for (c = contents->next; c; c = c->next) {
		if (a->pos.x > c->pos.x) {
			a->prev = c->prev;
			a->next = c;
			c->prev->next = a;
			c->prev = a;
			return;
		}
	} // Last
	a->prev = c;
	c->next = a;
}
void Actor::receive (Actor* a) {
	if (a->loc != id) {
		actor::global[a->loc]->release(a);
		a->loc = id;
	}
	if (a->active) a->deactivate();
	take_unsorted(a);
}
void Actor::release (Actor* a) {
	if (a == contents) {
		contents = a->next;
	}
	if (a->next) a->next->prev = a->prev;
	if (a->prev) a->prev->next = a->next;
}
void Actor::produce (Actor* a) {
	actor::global[loc]->receive(a);
}

bool Actor::has_body () {
	return type::def[type].nfixes > -1;
}

int Actor::get_room () {
	if (room::global[current_room]->in_room(this))
		return current_room;
	for (uint i=0; i < room::def[current_room].nneighbors; i++) {
		if (room::global[room::def[current_room].neighbors[i]]->in_room(this))
			return room::def[current_room].neighbors[i];
	}
	return -1;
}




#endif


