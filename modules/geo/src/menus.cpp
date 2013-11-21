#include "../inc/menus.h"
#include "../inc/camera.h"
#include "../../vis/inc/color.h"

namespace geo {
    using namespace core;
    using namespace vis;

    void Menu::activate () {
        if (!room.observer_count) {
            room.observe();
            Listener::activate();
        }
    }
    void Menu::deactivate () {
        if (room.observer_count) {
            room.forget();
            Listener::deactivate();
        }
    }

    bool Menu::active () {
        return room.observer_count > 0;
    }

    void Menu::Listener_cursor_pos (int x, int y) {
        Vec cursor_pos = dev_space
            ? camera->window_to_dev(x, y)
            : camera->window_to_hud(x, y);
        float lowest = INF;
        hovering = NULL;
        for (auto& res : room.residents) {
            Vec pos = res.Resident_get_pos();
            if (!pos.is_defined()) continue;
            const Rect& boundary = res.Resident_boundary();
            if (boundary.t < lowest) {
                if (boundary.covers(cursor_pos - pos)) {
                    lowest = boundary.t;
                    hovering = &res;
                }
            }
        }
        if (hovering)
            hovering->Resident_hover(cursor_pos);
    }
    bool Menu::Listener_button (int code, int action) {
        if (code == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
            if (hovering) {
                Vec pos = camera->window_to_hud(window->cursor_x, window->cursor_y);
                hovering->Resident_click(pos);
            }
        }
        return true;
    }

    void Text_Button_Base::set_text (std::string t) {
        text = t;
        boundary = Rect();
    }

    void Text_Button_Base::finish () { Resident::finish(); }

    Vec Text_Button_Base::Resident_get_pos () { return pos; }
    void Text_Button_Base::Resident_set_pos (Vec p) { pos = p; }
    Rect Text_Button_Base::Resident_boundary () {
        if (!boundary.is_defined()) {
            Vec size = text_size(text, font);
            Vec lt = size.scale(align+Vec(1,1)) / 2;
            Vec rb = size.scale(Vec(1,1)-align) / 2;
            boundary.l = lt.x;
            boundary.b = rb.y;
            boundary.r = rb.x;
            boundary.t = lt.y;
        }
        return boundary;
    }
    void Text_Button_Base::Resident_hover (Vec) { hovering = true; }
    void Text_Button_Base::Resident_click (Vec) { on_click(); }

    void Text_Button_Base::draw () {
        uint32 fg = hovering ? hover_color : color;
        uint32 bg = hovering ? hover_background_color : background_color;

        color_offset(pos);
        draw_color(bg);
        draw_solid_rect(boundary);
        draw_text(text, font, pos, align, fg);
    }

    void Button::Menu_Item_draw (Vec pos, Vec size) {
        uint32 bg = hovering() ? hover_background_color : background_color;

        color_offset(pos);
        draw_color(bg);
        draw_solid_rect(Rect(0, 0, size.x, size.y));
    }

    Vec VBox::Menu_Item_size (Vec area) {
        float height = 0;
        for (auto& c : contents) {
            Vec csize = c->Menu_Item_size(area - Vec(0, height));
            height += csize.y;
        }
        return Vec(area.x, height);
    }
    void VBox::Menu_Item_draw (Vec pos, Vec area) {
        float height = 0;
        for (auto& c : contents) {
            Vec csize = c->Menu_Item_size(area - Vec(0, height));
            height += csize.y;
            c->Menu_Item_draw(Vec(pos.x, height), Vec(area.x, csize.y));
        }
    }
    bool VBox::Menu_Item_hover (Vec pos, Vec area) {
        float height = 0;
        for (auto& c : contents) {
            Vec csize = c->Menu_Item_size(area - Vec(0, height));
            height += csize.y;
            if (c->Menu_Item_hover(Vec(pos.x, pos.y - height), Vec(area.x, csize.y)))
                return true;
        }
        return Button::Menu_Item_hover(pos, area);
    }
    bool VBox::Menu_Item_click (Vec pos, Vec area) {
        float height = 0;
        for (auto& c : contents) {
            Vec csize = c->Menu_Item_size(area - Vec(0, height));
            height += csize.y;
            if (c->Menu_Item_click(Vec(pos.x, pos.y - height), Vec(area.x, csize.y)))
                return true;
        }
        return Button::Menu_Item_click(pos, area);
    }

    Vec Text_Button2::Menu_Item_size (Vec area) {
        if (area != cached_area) {
            cached_area = area;
            cached_size = text_size(text, font, area.x);
        }
        return cached_size;
    }

    void Text_Button2::Menu_Item_draw (Vec pos, Vec size) {
        uint32 fg = hovering() ? hover_color : color;
        Button::Menu_Item_draw(pos, size);
        draw_text(text, font, pos, Vec(1, -1), fg, cached_area.x);
    }

} using namespace geo;

HACCABLE(Menu) {
    name("geo::Menu");
    attr("room", member(&Menu::room).optional());
    attr("dev_space", member(&Menu::dev_space).optional());
}

HACCABLE(Text_Button_Base) {
    name("geo::Text_Button_Base");
    attr("Resident", base<Resident>());
    attr("pos", member(&Text_Button_Base::pos).optional());
    attr("align", member(&Text_Button_Base::align).optional());
    attr("font", member(&Text_Button_Base::font).optional());
    attr("text", member(&Text_Button_Base::text).optional());
    attr("on_click", member(&Text_Button_Base::on_click).optional());
    attr("color", member(&Text_Button_Base::color).optional());
    attr("background_color", member(&Text_Button_Base::background_color).optional());
    attr("hover_color", member(&Text_Button_Base::hover_color).optional());
    attr("hover_background_color", member(&Text_Button_Base::hover_background_color).optional());
    finish([](Text_Button_Base& v){ v.finish();});
}

HACCABLE_TEMPLATE(<class Layer>, Text_Button<Layer>) {
    name([](){ return "geo::Text_Button<" + hacc::Type::CppType<Layer>().name() + ">"; });
    delegate(hcb::template base<Text_Button_Base>());
}
HCB_INSTANCE(Text_Button<vis::Hud>)
HCB_INSTANCE(Text_Button<vis::Dev>)

HACCABLE(Button) {
    name("geo::Button");
    attr("on_click", member(&Button::on_click).optional());
    attr("color", member(&Button::color).optional());
    attr("background_color", member(&Button::background_color).optional());
    attr("hover_color", member(&Button::hover_color).optional());
    attr("hover_background_color", member(&Button::hover_background_color).optional());
}

HACCABLE(VBox) {
    name("geo::VBox");
    attr("Button", base<Button>().optional());
    attr("contents", member(&VBox::contents));
}

HACCABLE(Text_Button2) {
    name("geo::Text_Button2");
    attr("Button", base<Button>().optional());
    attr("text", member(&Text_Button2::text));
    attr("font", member(&Text_Button2::font).optional());
}
