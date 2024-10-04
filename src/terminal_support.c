#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <curl/curl.h>
#include <curl/easy.h>

size_t get_terminal_emulator_name(void)
{
    /*
     * There are different "protocols", or "escape sequences", that can help 
     * determine if a termianl supports displaying pictures. So instead of 
     * trying to get the terminal emulator name,
     * and testing x amount of terminals, we can just get the protocol, and 
     * determine if the terminal emulator can display a picture.
    */


    // make this switch case ?
    char *terminal_emulator_protocol = getenv("TERM");

    #ifdef __linux__
        if (terminal_emulator_protocol && strcmp(terminal_emulator_protocol, "xterm-kitty") == 0) {
            fprintf(stdout, "Should be able to display pictures\n");
        }

        if (terminal_emulator_protocol && strcmp(terminal_emulator_protocol, "xterm-256-colors") == 0) {
            printf("Should display pictures \n");
        }

        /*
         * FIX: In kitty there is xterm, like in wezterm so altough they are not xterm but xterm-xxx they still get return 1 cant display pictures
        */

        if ((terminal_emulator_protocol && strcmp(terminal_emulator_protocol, "mlterm") == 0) 
            || (terminal_emulator_protocol && (strcmp(terminal_emulator_protocol, "xterm"))) == 0) {
            return 1;
        }
    #endif 


    #ifdef __APPLE__ 
        char * terminal_emulator_protocol_iterm = getenv("TERM_PROGRAMM");
        if (terminal_emulator_protocol_iterm && strcmp(terminal_emulator_protocol_iterm, "iTerm.app") == 0) {
            fprintf(stdout, "Should be able to display pictures\n");
        }

        if (terminal_emulator_protocol && strcmp(terminal_emulator_protocol, "xterm-kitty") == 0) {
            fprintf(stdout, "Should be able to display pictures\n");
        }

    #endif

    return 0;
}

