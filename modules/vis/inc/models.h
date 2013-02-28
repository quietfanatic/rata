#ifndef HAVE_VIS_MODELS_H
#define HAVE_VIS_MODELS_H

#include <stdexcept>
#include "../../util/inc/Vec.h"
#include "../../hacc/inc/haccable_standard.h"
#include "images.h"

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
            Skel::Seg* parent = NULL;  // Null if root, set after creation
            std::vector<Skel::Seg*> branches;
            Layout* layout;
            float z_offset;
        };

        hacc::named_vector<Skel::Seg> segs;
        Skel::Seg* root;
        Vec root_offset;
        hacc::named_vector<Pose> poses;

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

        std::string name;
        std::vector<App> apps;
    };

    struct Skin {
        struct App {
            Skel::Seg* target;
            std::vector<Texture*> textures;
        };
        
        std::vector<Skin::App> apps;
    };

    struct Model {
        struct Seg {
            Skin::App* skin = NULL;
            Pose::App* pose = NULL;

        };

        Skel* skel = NULL;
        std::vector<Model::Seg> segs;

        Model ();
        Model (Skel*);

        Vec offset_of (Skel::Seg* seg);
        void apply_pose (Pose*);
        void apply_pose_index (uint);
        void apply_skin (Skin*);

        void draw_seg (Model::Seg* ms, Skel::Seg* ss, Vec pos, bool fh, bool fv, float z);
        void draw (Vec pos, bool fliph = false, bool flipv = false, float z = 0);
    };


}


#endif
