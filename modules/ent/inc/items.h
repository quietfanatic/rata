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

    struct Item : Linkable<Item> {
         // If it's a Resident, reclude and NULL room
        virtual void Item_take (Biped*) = 0;
         // If it's a Resident, emerge
        virtual void Item_drop (Biped*) = 0;
         // Draw it as an inventory item (NYI)
        virtual void Item_draw (Vec pos) { }
    };

    struct Equipment : Item {
         // Use this to tweak stats
        virtual void Equipment_stats (BipedStats*) { }
         // If this returns NULL, it can't be equipped by this entity
        virtual vis::Skin* Equipment_skin (vis::Skel* target) = 0;
        virtual EquipementSlots Equipment_slots () = 0;
    };

}

#endif
