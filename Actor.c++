

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
	void debug_print ();
};

#else

Actor::Actor (actor::Def* def) :
	actor::Def(*def),
	id( def - actor::saved < actor::n_globals ? def - actor::saved : -1 ),
	active(false),
	awaiting_activation(false),
	contents(NULL),
	next(NULL),
	prev(NULL),
	next_active(NULL)
{
	printf("Creating actor %d=%d\n", (def - actor::saved < actor::n_globals ? def - actor::saved : -1), id);
}

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
	a->prev = NULL;
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
	actor::global[a->loc]->release(a);
	a->loc = id;
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
	if (room::global[loc]->in_room(this))
		return loc;
	for (uint i=0; i < room::def[loc].nneighbors; i++) {
		if (room::global[room::def[loc].neighbors[i]]->in_room(this))
			return room::def[loc].neighbors[i];
	}
	return -1;
}

void Actor::debug_print () {
	printf("%02d %08x: %02d %8.8s (% 8.4f % 8.4f) (% 8.4f % 8.4f) %08x%08x%08x %1d%1d %02d (%02d %02d) %02d\n",
		id, this, loc, type::def[type].name, pos.x, pos.y, vel.x, vel.y,
		facing, data, data2, active, awaiting_activation,
		contents ? contents->id : -2,
		prev ? prev->id : -2,
		next ? next->id : -2,
		next_active ? next_active->id : -2
	);
}



#endif


