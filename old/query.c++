
	
 // Find one object (by default solid) in the area given by world coords
 // This has false positives.
struct AreaChecker : public b2QueryCallback {
	b2Fixture* found;
	uint16 cat;
	bool ReportFixture (b2Fixture* fix) {
		if (fix->GetFilterData().categoryBits & cat) {
			found = fix;
			return false;
		}
		else {
			return true;
		}
	}
};
Object* check_area (float l, float t, float r, float b, uint16 cat) {
	AreaChecker checker;
	checker.found = NULL;
	checker.cat = cat;
	b2AABB aabb;
	if (l > r) SWAP(l, r)
	if (t > b) SWAP(t, b)
	aabb.lowerBound.Set(l, b);
	aabb.upperBound.Set(r, t);
	world->QueryAABB(&checker, aabb);
	if (checker.found)
		return (Object*) checker.found->GetBody()->GetUserData();
	else return NULL;
}

 // Find one object (by default solid) along the line in world coords
float32 LineChecker::ReportFixture (b2Fixture* fix, const b2Vec2& p, const b2Vec2& n, float32 f) {
	if (fix->GetFilterData().categoryBits & mask)
	if (fix->GetBody()->GetUserData() != owner)
	if (((Object*)fix->GetBody()->GetUserData())->active) {
		dbg_util("raytrace hit with cf %u, %u.\n", fix->GetFilterData().categoryBits, mask);
		if (f < frac) {
			hit = fix;
			frac = f;
			norm = n;
		}
		return frac;
	}
	return 1;
}

struct LineLooker : public b2RayCastCallback {
	uint16 mask;
	bool seen;
	float32 ReportFixture (b2Fixture* fix, const b2Vec2& p, const b2Vec2& n, float32 f) {
		if (fix->GetFilterData().categoryBits & mask) {
			seen = true;
			return 0;
		}
		else {
			return -1;
		}
	}
};

LineChecker check_line (Vec from, Vec to, uint16 mask, Object* owner) {
	LineChecker checker;
	checker.owner = owner;
	checker.mask = mask;
	checker.frac = 1;
	checker.hit = NULL;
	world->RayCast(&checker, from, to);
	return checker;
}
bool look_line (Vec from, Vec to, uint16 mask) {
	LineLooker looker;
	looker.mask = mask;
	looker.seen = false;
	world->RayCast(&looker, from, to);
	return looker.seen;
}







