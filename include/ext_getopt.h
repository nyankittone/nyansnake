// Header file for my stupid library for getopt().

/* This is a struct for storing information about a single command option.
   Functions in this library operate on entire arrays of these structs. */
typedef struct CommandOption {
    char *short_opts,
    *long_opt,
    *help_text;
} CommandOption;

char *getProgramName(char *argv0);
void printOptions(char *progname, CommandOption *opts_list, FILE *file);
void printHelp (
    char *program_name, char *description, CommandOption *opts_list, char *footnote
);
char *makeShortOpts(CommandOption *opts_list, size_t opts_list_length);
