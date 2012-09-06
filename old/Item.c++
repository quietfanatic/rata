


Item::Item (room::Def* loc, Vec pos, item::Def* def) :
	Spatial(type::item, loc, pos),
	def(def)
{ }
void Item::after_move () {
	if (rata->floor)
		rata->propose_action(Rata::action_equip, this, pos, 1);
}
void Item::draw () {
	if (def)
		draw::image(
			def->appearance,
			pos,
			def->world_frame
		);
}











