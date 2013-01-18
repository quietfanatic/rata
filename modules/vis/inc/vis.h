#ifndef HAVE_VIS_VIS_H
#define HAVE_VIS_VIS_H

#include <unordered_map>

#include "../../util/inc/Vec.h"
#include "../../util/inc/Rect.h"
#include "../../util/inc/organization.h"
#include "../../core/inc/resources.h"

#define PX (1/16.0)

namespace vis {

     // Only the barebones image file
    struct Image : Resource {
        uint tex = 0;
        Vec size;

        void unload ();
        void reload ();

        Image (std::string name);
        ~Image () { unload(); }
    };

     // Part of another image
    struct SubImg {
        Vec pos;
        Rect box;
        std::vector<Vec> misc;
    };

    void draw_img (Image* set, SubImg* sub, Vec p, bool fliph, bool flipv);


    struct Image_Drawer : Linkable<Image_Drawer> {
        virtual void draw () = 0;
        bool is_visible () { return is_linked(); }
        void appear ();
        void disappear ();
    };
    extern Links<Image_Drawer> image_drawers;
    struct Single_Image : Image_Drawer {
        virtual Image* img_image () = 0;
        virtual SubImg* img_sub () = 0;
        virtual Vec img_pos () = 0;
        virtual bool img_fliph () { return false; }
        virtual bool img_flipv () { return false; }
        void draw () {
            draw_img(img_image(), img_sub(), img_pos(), img_fliph(), img_flipv());
        }
    };

}

#endif
