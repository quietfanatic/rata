#ifdef HEADER

struct Spatial : Actor {
	room::Def* loc;
	Spatial* next;
	Spatial* prev;
	Vec pos;

	Spatial (int16 type, room::Def* loc, Vec pos);
	bool in_room (room::Def* r);
	void change_room ();
};

#else

Spatial::Spatial (int16 type, room::Def* loc, Vec pos) :
	Actor(type),
	loc(NULL),
	next(NULL),
	prev(NULL),
	pos(pos)
{
	if (loc) loc->receive(this);
}

bool Spatial::in_room (room::Def* r) {
	return in_rect(pos, r->pos, r->pos+vec(r->width, r->height));
}
void Spatial::change_room () {
	if (!in_room(loc))
	for (uint i=0; i < loc->n_neighbors; i++)
	if (in_room(loc->neighbors[i])) {
		loc->neighbors[i]->receive(this);
		return;
	}
}


#endif
