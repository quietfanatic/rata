#ifndef HAVE_ENT_ITEMS_H
#define HAVE_ENT_ITEMS_H

#include "../../util/inc/geometry.h"
#include "../../util/inc/organization.h"
#include "../../vis/inc/common.h"
#include "../../vis/inc/models.h"
#include "../../geo/inc/rooms.h"

namespace ent {

    struct Item;
    struct Biped_Stats;

    enum Equipment_Slots {
        HEAD = 1<<0,
        HAND = 1<<1,
        BODY = 1<<2,
        FEET = 1<<3,
    };

    struct Item_Def {
        std::string name = "<Mystery Item>";
        bool holdable = true;  // Can carry without equipping
        std::vector<vis::Skel*> equippable;
        Equipment_Slots slots = Equipment_Slots(0);
        vis::Skin* skin = NULL;
         // For drawing as an inventory item.
        vis::Texture* tex = NULL;
        vis::Frame* frame = NULL;

         // Inherit if you want to alter the equipper's stats
        virtual void Item_stats (Biped_Stats*) { }
    };

    struct Inventory {
        util::Links<Item> items;  // Passive list
    };

     // Objects that inherit both Item and Resident should never have
     //  both a owner and a room at the same time.
    struct Item : util::Link<Item> {
        Item_Def* def = NULL;
        Inventory* owner = NULL;
        Inventory* get_owner () const { return owner; }
        void set_owner (Inventory* inv) { link((owner = inv)->items); }
        Item () { }
    };

    struct ResItem_Def : Item_Def {
        vis::Texture* res_tex = NULL;
        vis::Frame* res_frame = NULL;
    };

    struct ResItem : Item, geo::Resident, vis::Drawn<vis::Sprites> {
        util::Vec pos;
        void Drawn_draw (vis::Sprites) override;
        void Resident_emerge () override { appear(); }
        void Resident_reclude () override { disappear(); }
        ResItem () { }
    };

}

#endif
