#ifndef HAVE_ENT_ITEMS_H
#define HAVE_ENT_ITEMS_H

#include "../../vis/inc/models.h"
#include "../../util/inc/organization.h"

namespace ent {

    struct Item_Def {
         // For drawing as an inventory item.
        vis::Texture* tex = NULL;
        vis::Frame* frame = NULL;
    };

    struct Item;

    struct Inventory {
        Links<Item> items;  // Passive list
    };


     // An Item can never simultaneously have a room and an owner.
     // Use set_owner to take and set_room then set_owner(NULL) to drop.
     //  (Can't call set_owner(NULL) in Resident_emerge because it might
     //   be dropped into an unobserved room.)
    struct Item : Link<Item>, geo::Resident {
        Item_Def* def = NULL;
        Inventory* owner = NULL;
        Inventory* get_owner () { return owner; }
        void set_owner (Inventory*);

        void Resident_emerge () override;
        void Resident_reclude () override;

        ~Item ();
    };

    enum Equipment_Slots {
        FEET = 1<<0,
        BODY = 1<<1,
        HEAD = 1<<2,
        HAND = 1<<3
    };

    struct Equipment_Def {
        vis::Texture* tex = NULL;
        vis::Frame* frame = NULL;
         // TODO: denote compatibility with different skeletons
        vis::Skin* skin = NULL;
        Equipment_Slots slots = Equipment_Slots(0);
    };

    struct Biped;
    struct Biped_Stats;

     // This is not a subclass of Item, because some things can be
     //  equipped but not carried unequipped.
    struct Equipment : Link<Equipment> {
        Equipment_Def* def = NULL;
        Biped* user = NULL;
        Biped* get_user () { return user; }
        void set_user (Biped*);
         // Use this to tweak stats
        virtual void Equipment_stats (Biped_Stats*) { }

        ~Equipment ();
    };

}

#endif
