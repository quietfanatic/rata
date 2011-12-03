

#ifdef HEADER

struct Actor {
	int16 type;
	bool active;
	bool awaiting_activation;
	Actor* next_global;
	Actor* next_active;

	Actor (int16 type);
	virtual void load (uint8** p) { }
	virtual void save (uint8** p) { }

	virtual void activate ();
	virtual void deactivate ();

	virtual void before_move () { }
	virtual void after_move () { }
	virtual void draw () { }
	
	bool has_body ();
	void debug_print ();
};

#else

Actor::Actor (int16 type) :
	type(type),
	active(false),
	awaiting_activation(false),
	next_global(global_actors),
	next_active(NULL)
{
	global_actors = this;
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
	dbg_actor("Activating %08x\n", this);
}
void Actor::deactivate () {
	if (!active) {
		printf("Warning: Attempted to deactivate an inactive actor.\n");
		return;
	}
	active = false;
	dbg_actor("Deactivating %08x\n", this);
}


bool Actor::has_body () {
	return type::def[type].nfixes > -1;
}

void Actor::debug_print () {
	printf("%08x: %8.8s %1d%1d %08x %08x\n",
		this, type::def[type].name,
		active, awaiting_activation,
		next_global, next_active
	);
}



#endif


