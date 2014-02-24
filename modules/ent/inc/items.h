#ifndef HAVE_ENT_ITEMS_H
#define HAVE_ENT_ITEMS_H

#include "../vis/models.h"

namespace ent {

    enum EquipmentSlots {
        FEET = 1<<0,
        BODY = 1<<1,
        HEAD = 1<<2,
        HAND = 1<<3
    };

    struct Owner {
        Links<Item>* items;  // Passive list
    };

     // An Item can never simultaneously have a room and an owner.
     // Use set_owner to take and set_room then set_owner(NULL) to drop.
    struct Item : Linkable<Item>, geo::Resident {
        Owner* owner;
        Owner* get_owner () { return owner; }
        void set_owner (Owner*);

        void Resident_emerge () override;
        void Resident_reclude () override;

        Item ();
        ~Item ();

         // For drawing as an inventory item.
        Texture* tex = NULL;
        Frame* frame = NULL;
    };

     // This is not a subclass of Item, because some things can be
     //  equipped but not carried unequipped.
    struct Equipment : Linkable<Equipment> {
         // Use this to tweak stats
        virtual void Equipment_stats (BipedStats*) { }
         // If this returns NULL, it can't be equipped by this entity
        virtual vis::Skin* Equipment_skin (vis::Skel* target) = 0;
        virtual EquipementSlots Equipment_slots () = 0;
    };

}

#endif
