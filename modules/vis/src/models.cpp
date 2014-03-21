#include "vis/inc/models.h"

#include <sstream>
#include "hacc/inc/everything.h"
#include "util/inc/debug.h"
#include "vis/inc/common.h"
using namespace util;

namespace vis {

    void Skel::finish () {
        for (auto& seg : segs) {
            auto n_branches = seg.branches.size();
            for (auto& frame : seg.layout->frames) {
                if (frame.points.size() < n_branches) {
                    throw hacc::X::Logic_Error(
                        "Skeleton error: Frame " + frame.name
                      + " of Seg " + seg.name
                      + " doesn't have enough points (" + std::to_string(frame.points.size())
                      + " < " + std::to_string(n_branches) + ")"
                    );
                }
            }
        }
    }

    uint Skel::seg_index (Seg* p) {
        uint r = p - segs.data();
        if (r >= segs.size()) {
            std::ostringstream ss;
            ss << "Segment " << p << " doesn't belong to Skel " << this;
            throw std::logic_error(ss.str());
        }
        return r;
    }

    size_t Skel::model_data_size () {
        return segs.size() * sizeof(Model::Seg);
    }

    static void reposition_segment (Model* model, Skel::Seg* ss, Vec pos) {
        Model::Seg* ms = &model->segs[model->skel->seg_index(ss)];
        ms->pos = pos;
        for (size_t i = 0; i < ss->branches.size(); i++) {
            Vec new_pos = pos + PX*ms->pose->frame->points[i] * ms->pose->scale;
            reposition_segment(model, ss->branches[i], new_pos);
        }
    }

    struct Draw {
        Model::Seg* seg;
        Texture* tex;
        float z;
    };
    int cmp_Draw (const void* a, const void* b) {
        float az = ((Draw*)a)->z;
        float bz = ((Draw*)b)->z;
         // Ascending order
        return (az > bz) - (az < bz);
    }

    void Model::draw (size_t n_skins, Skin** skins, Vec pos, Vec scale, float z) {
        if (!skel) {
            log("models", "Model has no associated skeleton");
            return;
        }
        log("models", "Drawing a model with %lu segs", skel->segs.size());
        reposition_segment(this, skel->root, skel->root_offset);
        size_t n_draws = 0;
        for (size_t i = 0; i < n_skins; i++) {
            n_draws += skins[i]->apps.size();
        }
        Draw draws [n_draws];
        size_t draw_i = 0;
        for (size_t i = 0; i < n_skins; i++) {
            for (auto& sa : skins[i]->apps) {
                draws[draw_i].seg = &segs[skel->seg_index(sa.target)];
                draws[draw_i].tex = sa.texture;
                draws[draw_i].z = sa.target->z_offset + sa.z_offset;
                draw_i++;
            }
        }
        qsort(draws, n_draws, sizeof(Draw), cmp_Draw);
        for (size_t i = 0; i < n_draws; i++) {
            draw_frame(
                draws[i].seg->pose->frame, draws[i].tex,
                pos + scale * draws[i].seg->pos * scale,
                scale * draws[i].seg->pose->scale, z
            );
        }
    }

    void Model::apply_pose (Pose* pose) {
        for (auto& app : pose->apps) {
            segs[skel->seg_index(app.target)].pose = &app;
        }
    }

    Model::Model (Skel* skel, char* data) : skel(skel), segs((Model::Seg*)data) {
        for (size_t i = 0; i < skel->segs.size(); i++) {
            segs[i].pose = NULL;
        }
    }

} using namespace vis;

HACCABLE(Skel) {
    name("vis::Skel");
    attr("segs", member(&Skel::segs));
    attr("root", member(&Skel::root));
    attr("root_offset", member(&Skel::root_offset));
    finish([](Skel& skel){ skel.finish(); });
}

HACCABLE(Skel::Seg) {
    name("vis::Skel::Seg");
    attr("name", member(&Skel::Seg::name));
    attr("branches", member(&Skel::Seg::branches).optional());
    attr("layout", member(&Skel::Seg::layout));
    attr("z_offset", member(&Skel::Seg::z_offset).optional());
}

HACCABLE(Pose) {
    name("vis::Pose");
    delegate(member(&Pose::apps));
}

HACCABLE(Pose::App) {
    name("vis::Pose");
    elem(member(&Pose::App::target));
    elem(member(&Pose::App::frame));
    elem(member(&Pose::App::scale).optional());
}

HACCABLE(Skin) {
    name("vis::Skin");
    attr("apps", member(&Skin::apps));
}

HACCABLE(Skin::App) {
    name("vis::Skin::App");
    elem(member(&Skin::App::target));
    elem(member(&Skin::App::texture));
    elem(member(&Skin::App::z_offset).optional());
}

