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
     // Dynamic things
    struct Model;
    struct ModelSegment;

    struct Pose {
        std::string name;
        SubImg subimg;
        bool fliph;
        bool flipv;
        std::vector<Vec> joints;
    };

    struct Segment {
        std::string name;
        std::vector<Segment*> subs;
        hacc::follow_ptr<std::vector<Pose>> poses;

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
        float z_shift;
        SkinSegment* covers;
    };

    struct ModelSegment {
        SkinSegment* skin;
        Pose* pose;
        Vec pos;

        void draw (Vec mpos, bool fliph, bool flipv);
    };

    struct Model {
        Skeleton* skeleton;
        std::vector<ModelSegment> model_segments;

        void reposition_segment (Segment* segment, Vec pos);
        
        void reposition () {
            reposition_segment(skeleton->root, skeleton->root_pos);
        }

        void apply_pose (Segment* segment, Pose* pose);
        void apply_preset (Preset* preset, Pose* pose);

        Vec position_of (Segment* segment) {
            return model_segments.at(skeleton->offset_of_segment(segment)).pos;
        }

        void draw (Vec pos, bool fliph = false, bool flipv = false);
    };


}


#endif
