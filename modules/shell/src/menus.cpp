#include "../inc/menus.h"
#include "../../geo/inc/camera.h"
#include "../../vis/inc/color.h"

namespace shell {
    using namespace core;
    using namespace vis;

    void Menu_Base::draw () {
        if (root) root->Menu_Item_draw(pos, size);
        color_offset(pos);
        draw_color(0x000000ff);
        draw_rect(Rect(0, 0, size.x, size.y));
    }

    void Menu_Base::hover (Vec cursor_pos) {
        if (Rect(0, 0, size.x, size.y).covers(cursor_pos - pos)) {
            root->Menu_Item_hover(cursor_pos - pos, size);
        }
    }
    bool Menu_Base::click (Vec cursor_pos) {
        if (Rect(0, 0, size.x, size.y).covers(cursor_pos - pos)) {
            root->Menu_Item_click(cursor_pos - pos, size);
            return true;
        }
        return false;
    }

    void Button::Menu_Item_draw (Vec pos, Vec size) {
        uint32 bg = hovering() ? hover_background_color : background_color;
        if (bg & 0xff) {
            color_offset(pos);
            draw_color(bg);
            draw_solid_rect(Rect(0, 0, size.x, size.y));
        }
    }

    Vec VBox::Menu_Item_size (Vec area) {
        if (cached_area != area) {
            cached_area = area;
            Vec size = Vec(0, 0);
            for (auto& c : contents) {
                Vec csize = c->Menu_Item_size(area - Vec(0, size.y));
                size.y += csize.y;
                if (csize.x > size.x) size.x = csize.x;
            }
            cached_size = size;
        }
        return cached_size;
    }
    void VBox::Menu_Item_draw (Vec pos, Vec area) {
        VBox::Menu_Item_size(area);
        Button::Menu_Item_draw(pos, area);
        float height = 0;
        for (auto& c : contents) {
            Vec csize = c->Menu_Item_size(area - Vec(0, height));
            height += csize.y;
            c->Menu_Item_draw(Vec(pos.x, pos.y + cached_size.y - height), Vec(area.x, csize.y));
        }
    }
    bool VBox::Menu_Item_hover (Vec pos, Vec area) {
        VBox::Menu_Item_size(area);
        float height = 0;
        for (auto& c : contents) {
            Vec csize = c->Menu_Item_size(area - Vec(0, height));
            height += csize.y;
            Vec cpos = pos - Vec(0, cached_size.y - height);
            if (Rect(0, 0, area.x, csize.y).covers(cpos)) {
                if (c->Menu_Item_hover(cpos, Vec(area.x, csize.y)))
                    return true;
            }
        }
        return Button::Menu_Item_hover(pos, area);
    }
    bool VBox::Menu_Item_click (Vec pos, Vec area) {
        VBox::Menu_Item_size(area);
        float height = 0;
        for (auto& c : contents) {
            Vec csize = c->Menu_Item_size(area - Vec(0, height));
            height += csize.y;
            Vec cpos = pos - Vec(0, cached_size.y - height);
            if (Rect(0, 0, area.x, csize.y).covers(cpos)) {
                if (c->Menu_Item_click(cpos, Vec(area.x, csize.y)))
                    return true;
            }
        }
        return Button::Menu_Item_click(pos, area);
    }

    Vec Text_Button::Menu_Item_size (Vec area) {
        if (area != cached_area) {
            cached_area = area;
            cached_size = text_size(text, font, area.x);
        }
        return cached_size;
    }

    void Text_Button::Menu_Item_draw (Vec pos, Vec size) {
        Button::Menu_Item_draw(pos, size);
        uint32 fg = hovering() ? hover_color : color;
        draw_text(text, font, pos, Vec(1, -1), fg, cached_area.x);
    }

} using namespace shell;

HACCABLE(Menu_Base) {
    name("shell::Menu_Base");
    attr("pos", member(&Menu_Base::pos).optional());
    attr("size", member(&Menu_Base::size).optional());
    attr("root", member(&Menu_Base::root).optional());
    attr("deactivate_on_click", member(&Menu_Base::deactivate_on_click).optional());
}

HACCABLE_TEMPLATE(<class Layer>, Menu<Layer>) {
    name([](){ return "shell::Menu<" + hacc::Type::CppType<Layer>().name() + ">"; });
    delegate(hcb::template base<Menu_Base>());
}
HCB_INSTANCE(Menu<vis::Hud>)
HCB_INSTANCE(Menu<vis::Dev>)

HACCABLE(Menu_Item) {
    name("shell::Menu_Item");
}

HACCABLE(Button) {
    name("shell::Button");
    attr("Menu_Item", base<Menu_Item>().optional());
    attr("on_click", member(&Button::on_click).optional());
    attr("color", member(&Button::color).optional());
    attr("background_color", member(&Button::background_color).optional());
    attr("hover_color", member(&Button::hover_color).optional());
    attr("hover_background_color", member(&Button::hover_background_color).optional());
}

HACCABLE(VBox) {
    name("shell::VBox");
    attr("Button", base<Button>().optional());
    attr("contents", member(&VBox::contents));
}

HACCABLE(Text_Button) {
    name("shell::Text_Button");
    attr("Button", base<Button>().optional());
    attr("text", member(&Text_Button::text));
    attr("font", member(&Text_Button::font).optional());
}

