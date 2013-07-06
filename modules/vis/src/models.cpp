
#include "../inc/models.h"
#include "../inc/sprites.h"
#include "../../hacc/inc/everything.h"
#include "../../core/inc/phases.h"
#include "../../core/inc/state.h"
#include "../../core/inc/commands.h"

namespace vis {

    void Skel::finish () {
        for (auto& seg : segs) {
            auto n_branches = seg.branches.size();
            for (auto& frame : seg.layout->frames) {
                if (frame.points.size() < n_branches) {
                    fprintf(stderr, "Skeleton error: Frame %s of Seg %s doesn't have enough points (%lu < %lu).\n",
                        frame.name.c_str(), seg.name.c_str(), frame.points.size(), n_branches
                    );
                    throw std::logic_error("Skeleton contained errors.");
                }
            }
            for (Skel::Seg* branch : seg.branches) {
                if (branch->parent) {
                    fprintf(stderr, "Skeleton error: Seg %s was claimed by multiple parents.\n", branch->name.c_str());
                    throw std::logic_error("Skeleton contained errors.");
                }
                branch->parent = branch;
            }
        }
    }

    uint Skel::seg_index (Seg* p) {
        uint r = p - segs.data();
        if (r < 0 || r >= segs.size()) {
            throw std::logic_error("A Segment was used with a Skeleton it doesn't belong to.");
        }
        return r;
    }

    void Model::draw_seg (Model::Seg* ms, Skel::Seg* ss, Vec pos, bool fh, bool fv, float z) {
        if (!ms->skin) return;
        if (!ms->pose) return;
        for (core::Texture* tex : ms->skin->textures) {
            Draws_Sprites::draw_sprite(
                ms->pose->frame, tex, pos,
                ms->pose->fliph?!fh:fh, ms->pose->flipv?!fv:fv,
                z + ss->z_offset
            );
        }
        for (Skel::Seg*& branch : ss->branches) {
            Vec pt = PX*ms->pose->frame->points[&branch - ss->branches.data()];
            if (fh) pt.x = -pt.x;
            if (fv) pt.y = -pt.y;
            draw_seg(&segs[skel->seg_index(branch)], branch, pos + pt, fh, fv, z);
        }
    }
    void Model::draw (Vec pos, bool fliph, bool flipv, float z) {
        if (!skel) return;
        draw_seg(
            &segs[skel->seg_index(skel->root)], skel->root,
            pos + skel->root_offset, fliph, flipv, z
        );
    }

    void Model::apply_skel (Skel* skel_) {
        skel = skel_;
        segs.resize(skel->segs.size());
    }

    void Model::apply_pose (Pose* pose) {
        for (auto& app : pose->apps) {
            segs[skel->seg_index(app.target)].pose = &app;
        }
    }
    void Model::apply_pose_index (uint i) {
        if (!skel) return;
        apply_pose(&skel->poses.at(i));
    }
    void Model::apply_skin (Skin* skin) {
        for (auto& app : skin->apps) {
            segs[skel->seg_index(app.target)].skin = &app;
        }
    }

    Model::Model () : skel(NULL), segs() { }
    Model::Model (Skel* skel) : skel(skel), segs(skel ? skel->segs.size() : 0) { }


    struct Model_Tester;
    static Model_Tester* model_tester;
    struct Model_Tester : core::Layer {
        Model_Tester () : core::Layer("D.M", "model_tester", false) {
            model_tester = this;
            model = Model(hacc::File("modules/ent/res/small.skel").data());
            model.apply_skin(hacc::File("modules/rata/res/rata-base.skin").data());
            model.apply_pose(model.skel->poses.named("stand"));
        }
        bool flip = false;
        Model model;
        void run () {
            model.draw(Vec(10, 4), flip, false, 0.5);
        }
    };

} using namespace vis;

HCB_BEGIN(Skel)
    name("vis::Skel");
    attr("segs", member(&Skel::segs));
    attr("root", member(&Skel::root));
    attr("root_offset", member(&Skel::root_offset));
    attr("poses", member(&Skel::poses));
    finish([](Skel& skel, hacc::Tree*) { skel.finish(); });
HCB_END(Skel)

HCB_BEGIN(Skel::Seg)
    name("vis::Skel::Seg");
    attr("name", member(&Skel::Seg::name));
    attr("branches", member(&Skel::Seg::branches).optional());
    attr("layout", member(&Skel::Seg::layout));
    attr("z_offset", member(&Skel::Seg::z_offset).optional());
HCB_END(Skel::Seg)

HCB_BEGIN(Pose)
    name("vis::Pose");
    elem(member(&Pose::name));
    elem(member(&Pose::apps));
HCB_END(Pose)

HCB_BEGIN(Pose::App)
    name("vis::Pose");
    elem(member(&Pose::App::target));
    elem(member(&Pose::App::frame));
    elem(member(&Pose::App::fliph).optional());
    elem(member(&Pose::App::flipv).optional());
HCB_END(Pose::App)

HCB_BEGIN(Skin)
    name("vis::Skin");
    attr("apps", member(&Skin::apps));
HCB_END(Skin)

HCB_BEGIN(Skin::App)
    name("vis::Skin::App");
    elem(member(&Skin::App::target));
    elem(member(&Skin::App::textures));
HCB_END(Skin::App)
