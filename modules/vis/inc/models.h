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
        std::vector<Vec> joints;
    };

    struct Segment {
        std::string name;
        std::vector<Segment*> subs;
        hacc::follow_ptr<std::vector<Pose>> poses;

        Pose* pose_named (std::string name) {
            for (auto& pose : *poses)
                if (pose.name == name) return &pose;
            throw std::logic_error("Segment Pose not found.");
        }
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

        Segment* segment_named (std::string name) {
            for (auto& p : segments)
                if (p.name == name) return &p;
            throw std::logic_error("Skeleton Segment not found.");
        }
        Preset* preset_named (std::string name) {
            for (auto& p : presets)
                if (p.name == name) return &p;
            throw std::logic_error("Skeleton Segment not found.");
        }
        uint offset_of_segment (Segment* p) {
            uint r = p - segments.data();
            if (r < 0 || r >= segments.size()) {
                throw std::logic_error("A Segment was used with a Skeleton it doesn't belong to.");
            }
        }
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
        SkinSegment* with;
    };

    struct ModelSegment {
        SkinSegment* skin;
        Pose* pose;
        Vec pos;
    };

    struct Model {
        Skeleton* skeleton;
        std::vector<ModelSegment> model_segments;

        void reposition_segment (Segment* segment, Vec pos) {
            ModelSegment& ms = model_segments.at(skeleton->offset_of_segment(segment));
            ms.pos = pos;
            if (!ms.pose) return;
            for (uint i = 0; i < segment->subs.size(); i++) {
                reposition_segment(segment->subs[i], pos + ms.pose->joints[i]);
            }
        }
        
        void reposition () {
            reposition_segment(skeleton->root, skeleton->root_pos);
        }

        void apply_pose (Segment* segment, Pose* pose) {
            model_segments.at(skeleton->offset_of_segment(segment)).pose = pose;
            reposition_segment(segment, Vec(0, 0));
        }
        void apply_preset (Preset* preset, Pose* pose) {
            for (auto& s_p : preset->segment_poses)
                model_segments.at(skeleton->offset_of_segment(s_p.first)).pose = s_p.second;
            reposition();
        }

        Vec position_of (Segment* segment) {
            return model_segments.at(skeleton->offset_of_segment(segment)).pos;
        }
    };


}


#endif
