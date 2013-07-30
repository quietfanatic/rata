
#include "../../core/inc/input.h"
#include "../../core/inc/commands.h"
#include "../../core/inc/window.h"
#include "../../vis/inc/common.h"
#include "../../vis/inc/color.h"
#include "../../vis/inc/text.h"

namespace shell {

    using namespace core;
    using namespace vis;

    struct Console : Key_Listener, Char_Listener, core::Console, Drawn<Dev> {
        bool is_active = false;
        std::string contents = console_help();
        std::string cli = "";
        uint cli_pos = 0;

        uint history_index = core::command_history.size();
        std::string stash_cli;

        bool ignore_a_trigger = false;

        Font* font = NULL;
        char trigger = '`';

        Console ();

        void Console_print (std::string message) override;
        bool hear_key (int keycode, int action) override;
        bool hear_char (int code, int action) override;
        void enter_console ();
        void exit_console ();
        void Drawn_draw (Dev) override;

    };

}

