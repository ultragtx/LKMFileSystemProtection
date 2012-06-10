#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#ifdef HAVE_SYS_FILE_H
#  include <sys/file.h>
#endif
#include <sys/stat.h>

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

#if defined (HAVE_STRING_H)
#  include <string.h>
#else /* !HAVE_STRING_H */
#  include <strings.h>
#endif /* !HAVE_STRING_H */

#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif

#include <time.h>

#ifdef READLINE_LIBRARY
#  include "readline.h"
#  include "history.h"
#else
#  include <readline/readline.h>
#  include <readline/history.h>
#endif

char *command_generator PARAMS((const char *, int));
char **fileman_completion PARAMS((const char *, int, int));

void initialize_readline () {
    rl_readline_name = "FileMan";
    
    //rl_attempted_completion_function = fileman_completion;
}


char **fileman_completion (text, start, end)
const char *text;
int start, end;
{
    char **matches;
    
    matches = (char **)NULL;
    
    /* If this word is at the start of the line, then it is a command
     to complete.  Otherwise it is the name of a file in the current
     directory. */
    if (start == 0)
        matches = rl_completion_matches (text, command_generator);
    
    return (matches);
}

char *
command_generator (text, state)
const char *text;
int state;
{
    static int list_index, len;
    char *name;
    
    /* If this is a new word to complete, initialize now.  This includes
     saving the length of TEXT for efficiency, and initializing the index
     variable to 0. */
    if (!state)
    {
        list_index = 0;
        len = strlen (text);
    }
    
    /* Return the next name which partially matches from the command list. */
    /*while (name = commands[list_index].name)
    {
        list_index++;
        
        if (strncmp (name, text, len) == 0)
            return (dupstr(name));
    }*/
    
    /* If no names matched, then return NULL. */
    return ((char *)NULL);
}


void main (argc, argv)
int argc;
char **argv;
{
    char *line, *s;
    
    //progname = argv[0];
    
    initialize_readline ();	/* Bind our completer. */
    
    /* Loop reading and executing lines until the user quits. */
    for ( ; ; )
    {
        line = readline ("FileMan: ");
        
        if (!line)
            break;
        
        /* Remove leading and trailing whitespace from the line.
         Then, if there is anything left, add it to the history list
         and execute it. */
        
        free (line);
    }
    exit (0);
}
