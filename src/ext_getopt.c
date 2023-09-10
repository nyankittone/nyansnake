/* This is a header file designed to extend the getopts library in POSIX
   with a couple more features. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <ext_getopt.h>

char *getProgramName(char *argv0) {
    if(!argv0) {
        errno = EINVAL;
        return NULL;
    }

    char *returned = strrchr(argv0,'/');
    return returned ? returned + 1:argv0;
}

void printOptions(char *program_name, CommandOption *opts_list, FILE *file) {
    fputs("TODO: Impliment printout for command options.\n",file);
}

void printHelp (
    char *program_name, char *description, CommandOption *opts_list, char *footnote
) {
    if(description) {
        char formed_program_name[snprintf(NULL,0,"\33[1;97m%s\33[m: ",program_name)];

        if(program_name) {
            sprintf(formed_program_name,"\33[1;97m%s\33[m: ",program_name);
        } else {
            formed_program_name[0] = '\0';
        }

        printf("%s%s\n",formed_program_name,description);
    }

    putchar('\n');
    printOptions(program_name,opts_list,stdout);

    if(footnote) {
        printf("\n%s\n",footnote);
    }
}

// Who knows if this code actually works??
char *makeShortOpts(CommandOption *opts_list, size_t opts_list_length) {
    char *returned = NULL;
    unsigned int short_opts_length,
    returned_length = 0;

    for(size_t i = 0; i < opts_list_length; i++) {
        // get length of short_opts
        short_opts_length = strlen(opts_list[i].short_opts);

        // realloc() returned to accomodate that length
        if(!short_opts_length) {
            continue;
        }

        returned_length += short_opts_length;
        returned = realloc(returned,sizeof(char) * (returned_length + 1));

        // strcat() contents of short_opts into returned
        strcat(returned,opts_list[i].short_opts);
    }

    return returned;
}
