#ifndef HAVE_VIS_MODELS_H
#define HAVE_VIS_MODELS_H

#include <stdexcept>
#include "hacc/inc/haccable_standard.h"
#include "util/inc/geometry.h"
#include "vis/inc/images.h"

namespace vis {

     // Static things
    struct Pose;
    struct Skel;
    struct Skin;
     // Dynamic things
    struct Model;

    struct Skel {
        struct Seg {
            std::string name;
            std::vector<Skel::Seg*> branches;
            Layout* layout;
            float z_offset;

            Skel::Seg* parent = NULL;  // Null if root, set after creation
        };

        hacc::named_vector<Skel::Seg> segs;
        Skel::Seg* root;
        util::Vec root_offset;

        uint seg_index (Skel::Seg* p);

        void finish ();

        size_t model_data_size ();
    };

    struct Pose {
        struct App {
            Skel::Seg* target;
            Frame* frame;  // Must belong to the target's layout
            util::Vec scale = util::Vec(1, 1);
        };

        std::vector<App> apps;
    };

    struct Skin {
        struct App {
            Skel::Seg* target;
            vis::Texture* texture;
            float z_offset = 0;
        };

        std::vector<Skin::App> apps;
    };

    struct Model {
        struct Seg {
            Pose::App* pose;
            util::Vec pos;
        };

        Skel* skel = NULL;
        Model::Seg* segs;

        Model (Skel*, char*);

        util::Vec offset_of (Skel::Seg* seg);
        void apply_pose (Pose*);

         // TODO: make this better
        void draw (size_t n_skins, Skin** skins, util::Vec pos, util::Vec scale = util::Vec(1, 1), float z = 0.5);
    };

}


#endif
