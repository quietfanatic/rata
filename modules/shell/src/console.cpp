#include <algorithm>  // std::move
#include "../../core/inc/commands.h"
#include "../../core/inc/window.h"
#include "../../vis/inc/common.h"
#include "../../vis/inc/color.h"
#include "../../vis/inc/text.h"

namespace shell {

    using namespace core;
    using namespace vis;

    struct Console;
    Console* console = NULL;
    struct Console : core::Console, Drawn<Dev>, core::Listener {
        std::string contents = console_help();
        std::string cli = "";
        uint cli_pos = 0;

        uint history_index = core::command_history.size();
        std::string stash_cli;

        Font* font = NULL;
         // We need to skip one frame of input when the console turns on,
         //  otherwise the hotkey that enabled it will be heard.
        bool wait_for_draw = false;
         // Tab completion FTW
        std::vector<std::string> completion_matches;

        static CE size_t max_size = 2<<15;

        Console () { if (!console) console = this; }
        ~Console () { if (console == this) console = NULL; }

        void Console_print (std::string message) override {
            contents += message;
        }

        bool Listener_key (int keycode, int action) override {
            if (action == GLFW_PRESS) {
                switch (keycode) {
                    case GLFW_KEY_ESC: {
                        exit_console();
                        break;
                    }
                    case GLFW_KEY_LEFT: {
                        if (cli_pos > 0) cli_pos--;
                        break;
                    }
                    case GLFW_KEY_RIGHT: {
                        if (cli_pos < cli.size()) cli_pos++;
                        break;
                    }
                    case GLFW_KEY_UP: {
                        if (history_index > 0) {
                            if (history_index == core::command_history.size()) {
                                stash_cli = cli;
                            }
                            history_index--;
                            cli = core::command_history[history_index];
                            cli_pos = cli.size();
                        }
                        break;
                    }
                    case GLFW_KEY_DOWN: {
                        if (history_index < core::command_history.size()) {
                            history_index++;
                            if (history_index == core::command_history.size())
                                cli = stash_cli;
                            else
                                cli = core::command_history[history_index];
                            cli_pos = cli.size();
                        }
                        break;
                    }
                    case GLFW_KEY_HOME: {
                        cli_pos = 0;
                        break;
                    }
                    case GLFW_KEY_END: {
                        cli_pos = cli.size();
                        break;
                    }
                    case GLFW_KEY_ENTER: {
                        print_to_console(cli + "\n");
                        command_from_string(cli);
                        cli = "";
                        cli_pos = 0;
                        stash_cli = "";
                        history_index = core::command_history.size();
                        break;
                    }
                    case GLFW_KEY_BACKSPACE: {
                        if (cli_pos) {
                            cli = cli.substr(0, cli_pos - 1)
                                + cli.substr(cli_pos);
                            cli_pos--;
                        }
                        break;
                    }
                    case GLFW_KEY_DEL: {
                        if (cli_pos < cli.size()) {
                            cli = cli.substr(0, cli_pos)
                                + cli.substr(cli_pos + 1);
                        }
                        break;
                    }
                    case GLFW_KEY_TAB: {
                         // Completion of a command
                        size_t start = 0;
                        while (start < cli_pos && cli[start] == ' ')
                            start++;
                        size_t end = cli_pos;
                        bool dont_complete = false;
                        for (size_t i = start; i < end; i++) {
                            if (cli[i] == ' ') {
                                dont_complete = true;
                                break;
                            }
                        }
                        if (dont_complete) break;
                        auto cmd_part = cli.substr(start, end - start);
                        if (!completion_matches.empty()) {
                            Console_print(cli + "\n");
                            for (auto s : completion_matches) {
                                Console_print(s + " ");
                            }
                            Console_print("\n");
                        }
                        else {
                             // Find all commands beginning with this
                            for (auto& p : commands_by_name()) {
                                if (p.first.size() >= cmd_part.size()) {
                                    for (size_t i = 0; i < cmd_part.size(); i++) {
                                        if (p.first[i] != cmd_part[i])
                                            goto no_match;
                                    }
                                    completion_matches.push_back(p.first);
                                }
                                no_match: { }
                            }
                             // One match?  Complete it.
                            if (completion_matches.size() == 1) {
                                cli.replace(start, end - start, completion_matches[0] + " ");
                                cli_pos = completion_matches[0].size() + 1;
                            }
                             // Multiple matches?  Fill as much as possible.
                            else if (completion_matches.size() > 1) {
                                for (;;) {
                                    char common = completion_matches[0][cmd_part.size()];
                                    for (auto s : completion_matches) {
                                        if (s.size() < cmd_part.size() || s[cmd_part.size()] != common) {
                                            goto done_adding;
                                        }
                                    }
                                }
                                done_adding:
                                cli.replace(start, end - start, cmd_part);
                                cli_pos = cmd_part.size();
                            }
                        }
                        return true;
                    }
                    default: break;
                }
                completion_matches.clear();
                return true;
            }
            return false;
        }
        bool Listener_char (int code, int action) override {
            if (wait_for_draw) return false;
            if (glfwGetKey(GLFW_KEY_LCTRL) || glfwGetKey(GLFW_KEY_RCTRL)) {
                switch (code) {
                    case 'd': {
                        exit_console();
                        break;
                    }
                    case 'c': {
                        cli = "";
                        cli_pos = 0;
                        break;
                    }
                    case 'u': {
                        cli = cli.substr(cli_pos);
                        cli_pos = 0;
                        break;
                    }
                    default: break;
                }
            }
            else if (code < 256) {
                cli = cli.substr(0, cli_pos)
                    + std::string(1, code)
                    + cli.substr(cli_pos);
                cli_pos++;
            }
            completion_matches.clear();
            return true;
        }
        void enter_console () {
            if (active) return;
            wait_for_draw = true;
            Listener::activate();
            appear();
        }
        void exit_console () {
            if (!active) return;
            Listener::deactivate();
            disappear();
        }
        void Drawn_draw (Dev r) override {
            wait_for_draw = false;
            if (!font) return;
             // Darken background
            color_offset(Vec(0, 0));
            draw_color(0x000000cf);
            draw_solid_rect(Rect(0, 0, core::window->width*PX, core::window->height*PX));
             // Draw text
            Vec cli_size = draw_text(cli + " ", font, Vec(1, 0)*PX, Vec(1, -1), 0x7fff00ff, core::window->width*PX);
            Vec cursor_pos = get_glyph_pos(cli, font, cli_pos, Vec(1, -1), core::window->width*PX);
            if (window->frames_drawn % 40 < 20) {
                draw_text("_", font, Vec(1*PX + cursor_pos.x, cli_size.y - cursor_pos.y - font->line_height*PX - 2*PX), Vec(1, -1), 0xffffffff);
            }
            draw_text(contents, font, Vec(1*PX, cli_size.y), Vec(1, -1), 0x00ff00ff, core::window->width*PX);
        }
    };
} using namespace shell;

HACCABLE(shell::Console) {
    name("shell::Console");
    attr("font", member(&shell::Console::font));
}

void _open_console () {
    if (console) console->enter_console();
}
core::New_Command _open_console_cmd ("open_console", "Open the in-game console.", 0, _open_console);

void _exit () {
    if (console) console->exit_console();
}
core::New_Command _exit_cmd ("exit", "Close the in-game console.", 0, _exit);
