#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>


size_t get_terminal_emulator_protocol(void)
{
    /*
     * There are different "protocols", or "escape sequences", that can help
     * determine if a termianl supports displaying pictures. So instead of
     * trying to get the terminal emulator name,
     * and testing x amount of terminals, we can just get the protocol, and
     * determine if the terminal emulator can display a picture.
    */


    // more protocols
    char *terminal_emulator_protocol = getenv("TERM");

    #ifdef __linux__
        if (terminal_emulator_protocol && strcmp(terminal_emulator_protocol, "xterm-256color") == 0) {
            return 0;
        }

        if ((terminal_emulator_protocol && strcmp(terminal_emulator_protocol, "mlterm") == 0)
            || (terminal_emulator_protocol && (strcmp(terminal_emulator_protocol, "xterm"))) == 0) {
            // fprintf(stdout, "\nYour terminal emulator protocol cannot display pictures!\n");
            return 1;
        }
    #endif


    #ifdef __APPLE__
        char * terminal_emulator_protocol_macOS = getenv("TERM_PROGRAMM");
        if (terminal_emulator_protocol_macOS && strcmp(terminal_emulator_protocol_macOS, "iTerm.app") == 0) {
            return 0;
        }

    #endif

    if (terminal_emulator_protocol && strcmp(terminal_emulator_protocol, "xterm-kitty") == 0) {
        return 0;
    }
    
    return 0;
}
