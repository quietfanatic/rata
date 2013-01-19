#ifndef HAVE_VIS_MODELS_H
#define HAVE_VIS_MODELS_H

#include <stdexcept>
#include "../../util/inc/Vec.h"
#include "../../hacc/inc/haccable_pointers.h"
#include "images.h"

namespace vis {
     // Static things
    struct Pose;
    struct Skel;
    struct Skin;
     // Dynamic things
    struct Model;

    struct Pose {
        struct App;

        std::string name;
        std::vector<App> apps;
    };

    struct Skel : Resource {
        struct Seg;

        std::vector<Skel::Seg> segs;
        Skel::Seg* root;
        Vec root_offset;
        std::vector<Pose> poses;

        Skel::Seg* seg_named (std::string name);
        Pose* pose_named (std::string name);
        uint seg_index (Segment* p);

         // as Resource
        void verify ();
        void reload ();
        Skel (std::string name);
        Skel ();
        ~Skel ();
    };
    struct Skel::Seg {
        std::string name;
        Skel::Seg* parent;  // Null if root, set after creation
        std::vector<Skel::Seg*> branches;
        Layout* layout;
        float z_offset;
    };
    struct Pose::App {
        Skel::Seg* target;
        SubImg* subimg;
        bool fliph;
        bool flipv;
    };

    struct Skin : Resource {
        struct Map;
        struct App;
        
        std::vector<Skin::Map> maps;
        std::vector<Skin::Seg> apps;
         // Resource
        void reload ();
        Skin ();
        Skin (std::string name);
    };

    struct Skin::Map {
        Image* image;
        Layout* layout;
        Vec img_offset = Vec(0, 0);
        float z_offset = 0.f;
    };

    struct Skin::App {
        Skel::Seg* target;
        Skin::Map* map;
        Skin::Seg* also = NULL;
    };


    struct Model {
        struct Seg;

        Skel* skel = NULL;
        std::vector<Model::Seg> segs;

        Model ();
        Model (Skeleton*);

        Vec offset_of (Skel::Seg* segment);
        void apply_pose_seg (Skel::Seg*, Pose::Seg*);
        void apply_pose (Pose*);
        void apply_skin_seg (Skin::Seg*);
        void apply_skin (Skin*);

        void draw (Vec pos, bool fliph = false, bool flipv = false, float z = 0);
    };

    struct Model::Seg {
        Skin::Seg* skin_seg = NULL;
        Pose::Seg* pose_seg = NULL;
        bool fliph = false;
        bool flipv = false;

        void draw (Segment* seg, Vec mpos, bool fh, bool fv, float z);
    };



}


#endif
