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
    prepare([](Equipment_Slots& slots, hacc::Tree tree){ });
    fill([](Equipment_Slots& slots, hacc::Tree tree){
        int s = 0;
        for (auto& m : tree.as<hacc::Array>()) {
            auto n = m.as<std::string>();
            if (n == "head") s |= HEAD;
            if (n == "hand") s |= HAND;
            if (n == "body") s |= BODY;
            if (n == "feet") s |= FEET;
        }
        slots = Equipment_Slots(s);
    });
    to_tree([](const Equipment_Slots& slots){
        hacc::Array r;
        if (slots & HEAD) r.push_back(hacc::Tree("head"));
        if (slots & HAND) r.push_back(hacc::Tree("hand"));
        if (slots & BODY) r.push_back(hacc::Tree("body"));
        if (slots & FEET) r.push_back(hacc::Tree("feet"));
        return hacc::Tree(std::move(r));
    });
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

