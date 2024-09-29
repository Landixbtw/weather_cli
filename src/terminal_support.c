#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <curl/curl.h>
#include <curl/easy.h>

/* This is 50/50 me and claude.ai */
char *get_terminal_emulator_name(void)
{
    /*
    * We can start by getting the parent process ID of our program.
    * Then, we can read the command line of that parent process from the /proc filesystem.
    * The command line often (but not always) includes the name of the terminal emulator.
    */

    char *terminal_name;
    // *p*arent *p*rocess *id*
    pid_t ppid = getppid();

    // TODO: This gives the shell not the terminal emulator name 

    #ifdef __linux__
        char proc_path[256];

        snprintf(proc_path, sizeof(proc_path), "/proc/%d/cmdline", ppid);
        // printf("proc_path: %s\n", proc_path);


        FILE *f = fopen(proc_path, "r");
        if (f == NULL) {
            perror("Failed to open proc file\n");
            return NULL;
        }


        char *line = NULL;
        size_t len = 0;
        ssize_t read = getline(&line, &len, f);
        fclose(f);

        if (read == -1) {
            free(line);
            return NULL;
        }

        // Extract the terminal emulator name from the command line
        // This part will depend on the specific format of your system
        terminal_name = strrchr(line, '/');
        if (terminal_name) {
            terminal_name++; // Move past the '/'
        } else {
            terminal_name = line;
        }

        // Remove any arguments
        char *space = strchr(terminal_name, ' ');
        if (space) *space = '\0';

        // printf("terminal name: %s\n", terminal_name);

    #endif 


    #ifdef __APPLE__ 

        fprintf(stdout, "PPID: %i\n", ppid);
        printf("PID APPLE: %i\n", getpid());

    #endif

    return terminal_name;
}

