
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
    struct Model_Tester : core::Layer, core::Stateful {
        Model_Tester () : core::Layer("D.M", "model_tester", false) { model_tester = this; }
        bool flip = false;
        Model model;
        void run () {
            model.draw(Vec(10, 4), flip, false, 0.5);
        }
        void start () {
            model = Model(hacc::reference_file<Skel>("modules/ent/res/small.skel"));
            model.apply_skin(hacc::reference_file<Skin>("modules/rata/res/rata-base.skin"));
            model.apply_pose(model.skel->poses.named("stand"));
        }
    };

} using namespace vis;

HCB_BEGIN(Skel)
    type_name("vis::Skel");
    attr("segs", member(&Skel::segs));
    attr("root", member(&Skel::root));
    attr("root_offset", member(&Skel::root_offset));
    attr("poses", member(&Skel::poses));
    finish([](Skel& skel) { skel.finish(); });
HCB_END(Skel)

HCB_BEGIN(Skel::Seg)
    type_name("vis::Skel::Seg");
    attr("name", member(&Skel::Seg::name));
    attr("branches", member(&Skel::Seg::branches)(optional));
    attr("layout", member(&Skel::Seg::layout));
    attr("z_offset", member(&Skel::Seg::z_offset, def(0.f)));
HCB_END(Skel::Seg)

HCB_BEGIN(Pose)
    type_name("vis::Pose");
    elem(member(&Pose::name));
    elem(member(&Pose::apps));
HCB_END(Pose)

HCB_BEGIN(Pose::App)
    type_name("vis::Pose");
    elem(member(&Pose::App::target));
    elem(member(&Pose::App::frame));
    elem(member(&Pose::App::fliph, def(false)));
    elem(member(&Pose::App::flipv, def(false)));
HCB_END(Pose::App)

HCB_BEGIN(Skin)
    type_name("vis::Skin");
    attr("apps", member(&Skin::apps));
HCB_END(Skin)

HCB_BEGIN(Skin::App)
    type_name("vis::Skin::App");
    elem(member(&Skin::App::target));
    elem(member(&Skin::App::textures));
HCB_END(Skin::App)

struct MT_Load_Command : Command {
    Skel* skel;
    void operator() () {
        model_tester->model = Model(skel);
    }
};
HCB_BEGIN(MT_Load_Command)
    base<Command>("mt_load");
    command_description<MT_Load_Command>("Load a skeleton into the model_test layer.\nUnload with 'mt_load null'");
    elem(member(&MT_Load_Command::skel));
HCB_END(MT_Load_Command)

struct MT_Skin_Command : Command {
    Skin* skin;
    void operator() () {
        model_tester->model.apply_skin(skin);
    }
};
HCB_BEGIN(MT_Skin_Command)
    base<Command>("mt_skin");
    command_description<MT_Skin_Command>("Apply a skin to the model_test layer");
    elem(member(&MT_Skin_Command::skin));
HCB_END(MT_Skin_Command)

struct MT_Pose_Command : Command {
    std::string name;
    void operator() () {
        model_tester->model.apply_pose(model_tester->model.skel->poses.named(name));
    }
};

HCB_BEGIN(MT_Pose_Command)
    base<Command>("mt_pose");
    command_description<MT_Pose_Command>("Apply a pose to the model_test layer by name");
    elem(member(&MT_Pose_Command::name));
HCB_END(MT_Pose_Command)

struct MT_Flip_Command : Command {
    void operator() () {
        model_tester->flip = !model_tester->flip;
    }
};
HCB_BEGIN(MT_Flip_Command)
    base<Command>("mt_flip");
    command_description<MT_Pose_Command>("Flip the model tester horizontally");
    empty();
HCB_END(MT_Flip_Command)

HCB_BEGIN(Model_Tester)
    type_name("vis::Model_Tester");
    base<core::Stateful>("Model_Tester");
    empty();
HCB_END(Model_Tester)
