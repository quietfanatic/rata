#ifndef HAVE_VIS_MODELS_H
#define HAVE_VIS_MODELS_H

#include <stdexcept>
#include "../../hacc/inc/haccable_standard.h"
#include "../../util/inc/geometry.h"
#include "images.h"

namespace vis {
    using namespace util;

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
        Vec root_offset;

        uint seg_index (Skel::Seg* p);

        void finish ();
    };

    struct Pose {
        struct App {
            Skel::Seg* target;
            Frame* frame;  // Must belong to the target's layout
            bool fliph;
            bool flipv;
        };

        std::vector<App> apps;
    };

    struct Skin {
        struct App {
            Skel::Seg* target;
            std::vector<vis::Texture*> textures;
        };

        std::vector<Skin::App> apps;
    };

    struct Model {
        struct Seg {
            Pose::App* pose;
        };

        Skel* skel = NULL;
        Model::Seg* segs;

        Model (Skel*, Model::Seg*);

        Vec offset_of (Skel::Seg* seg);
        void apply_pose (Pose*);

         // TODO: make this better
        void draw_seg (Skin* skin, Skel::Seg* ss, Vec pos, bool fh, bool fv, float z);
        void draw (Skin* skin, Vec pos, bool fliph = false, bool flipv = false, float z = 0);
    };

}


#endif
