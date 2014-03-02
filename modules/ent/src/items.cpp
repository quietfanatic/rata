#include "ent/inc/items.h"

#include <string>
#include <vector>
#include "hacc/inc/everything.h"
#include "vis/inc/models.h"

namespace ent {

    void ResItem::Drawn_draw (vis::Sprites) {
        if (!def || !room) return;
        auto ridef = static_cast<ResItem_Def*>(def);
        vis::draw_frame(ridef->res_frame, ridef->res_tex, pos);
    }

    Inventory::~Inventory () {
        for (auto& i : items)
            i.set_owner(NULL);
    }

} using namespace ent;

HACCABLE(Equipment_Slots) {
    name("ent::Equipment_Slots");
    delegate(value_funcs<std::vector<std::string>>(
        [](const Equipment_Slots& s) {
            std::vector<std::string> r;
            if (s & HEAD) r.push_back("head");
            if (s & HAND) r.push_back("hand");
            if (s & BODY) r.push_back("body");
            if (s & FEET) r.push_back("feet");
            return r;
        }, [](Equipment_Slots& s, std::vector<std::string> names){
            int r = 0;
            for (auto n : names) {
                if (n == "head") r |= HEAD;
                if (n == "hand") r |= HAND;
                if (n == "body") r |= BODY;
                if (n == "feet") r |= FEET;
            }
            s = Equipment_Slots(r);
        }
    ).narrow());
}

HACCABLE(Item_Def) {
    name("ent::Item_Def");
    attr("name", member(&Item_Def::name).optional());
    attr("holdable", member(&Item_Def::holdable).optional());
    attr("equippable", member(&Item_Def::equippable).optional());
    attr("slots", member(&Item_Def::slots).optional());
    attr("skin", member(&Item_Def::skin).optional());
    attr("tex", member(&Item_Def::tex).optional());
    attr("frame", member(&Item_Def::frame).optional());
}

HACCABLE(Inventory) {
    name("ent::Inventory");
}

HACCABLE(Item) {
    name("ent::Item");
    attr("def", member(&Item::def));
    attr("owner", value_methods(&Item::get_owner, &Item::set_owner).optional());
}

HACCABLE(ResItem_Def) {
    name("ent::ResItem_Def");
    attr("Item_Def", base<Item_Def>().collapse());
    attr("res_tex", member(&ResItem_Def::res_tex).optional());
    attr("res_frame", member(&ResItem_Def::res_frame).optional());
}

HACCABLE(ResItem) {
    name("ent::ResItem");
    attr("Item", base<Item>().collapse());
    attr("Resident", base<geo::Resident>().collapse());
}

