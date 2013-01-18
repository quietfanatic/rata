#ifndef HAVE_VIS_MODELS_H
#define HAVE_VIS_MODELS_H

#include <stdexcept>
#include "../../util/inc/Vec.h"
#include "../../hacc/inc/haccable_pointers.h"
#include "../inc/vis.h"

namespace vis {
     // Static things
    struct Pose;
    struct Segment;
    struct Preset;
    struct Skeleton;
    struct SkinSegment;
    struct Skin;
     // Dynamic things
    struct Model;
    struct ModelSegment;

    struct Pose {
        std::string name;
        SubImg subimg;
        bool fliph;
        bool flipv;
        std::vector<Vec> joints;
        float z_offset;
    };

    struct Segment {
        std::string name;
        std::vector<Segment*> subs;
        std::vector<Pose>* poses;
        float z_offset;

        Pose* pose_named (std::string name);
    };

    struct Preset {
        std::string name;
        std::vector<std::pair<Segment*, Pose*>> segment_poses;
    };

    struct Skeleton : Resource {
        std::vector<Segment> segments;
        Segment* root;
        Vec root_pos;
        std::vector<std::vector<Pose>> pose_lists;
        std::vector<Preset> presets;

        Segment* segment_named (std::string name);
        Preset* preset_named (std::string name);
        uint offset_of_segment (Segment* p);
         // Resource
        void reload ();
        Skeleton ();
        Skeleton (std::string name);
        ~Skeleton ();
    };

    struct SkinSegment {
        Image* image;
        Vec subimg_offset;
        float z_offset;
        SkinSegment* covers;
    };

    struct Skin : Resource {
        std::vector<std::pair<Segment*, SkinSegment>> segments;
         // Resource
        void reload ();
        Skin ();
        Skin (std::string name);
    };

    struct ModelSegment {
        SkinSegment* skin = NULL;
        Pose* pose = NULL;
        Vec pos;

        void draw (Vec mpos, bool fliph, bool flipv);
    };

    struct Model {
        Skeleton* skeleton = NULL;
        std::vector<ModelSegment> model_segments;

        Model ();
        Model (Skeleton*);

        void reposition_segment (Segment*, Vec);
        void reposition () {
            if (!skeleton) return;
            reposition_segment(skeleton->root, skeleton->root_pos*PX);
        }
        Vec position_of (Segment* segment) {
            if (!skeleton) return Vec();
            return model_segments.at(skeleton->offset_of_segment(segment)).pos;
        }

        void apply_pose (Segment*, Pose*);
        void apply_preset (Preset*);
        void apply_segment_skin (Segment*, SkinSegment*);
        void apply_skin (Skin*);

        void draw (Vec pos, bool fliph = false, bool flipv = false);
    };


}


#endif
