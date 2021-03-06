#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "cat.h"
#include "for.h"
#include "gosub.h"
#include "goto.h"
#include "if.h"
#include "input.h"
#include "keyword.h"
#include "let.h"
#include "list.h"
#include "load.h"
#include "new.h"
#include "print.h"
#include "rem.h"
#include "run.h"
#include "safemem.h"
#include "save.h"
#include "stringutil.h"

#define MAX_ID 10

static keyword keywords[] =
{
    { "CAT", KWFL_OK_IN_REPL, &cat_parse },
    { "FOR", KWFL_OK_IN_STMT, &for_parse },
    { "GOSUB", KWFL_OK_IN_STMT, &gosub_parse },
    { "GOTO", KWFL_OK_IN_STMT, &goto_parse },
    { "IF", KWFL_OK_IN_STMT, &if_parse },
    { "INPUT", KWFL_OK_IN_STMT, &input_parse },
    { "LET", KWFL_OK_IN_STMT | KWFL_OK_IN_REPL, &let_parse },
    { "LIST", KWFL_OK_IN_REPL, &list_parse },
    { "LOAD", KWFL_OK_IN_REPL, &load_parse },
    { "NEXT", KWFL_OK_IN_STMT, &next_parse },
    { "NEW", KWFL_OK_IN_REPL, &new_parse },
    { "PRINT", KWFL_OK_IN_STMT | KWFL_OK_IN_REPL, &print_parse },
    { "REM", KWFL_OK_IN_STMT | KWFL_OK_IN_REPL, &rem_parse },
    { "RUN", KWFL_OK_IN_REPL, &run_parse },
    { "RETURN", KWFL_OK_IN_STMT, &return_parse },
    { "SAVE", KWFL_OK_IN_REPL, &save_parse },

    { NULL, 0 }
};

/* Find a keyword in the table. Returns a pointer to the keyword
 * or NULL if there is no match.
 */
keyword *kw_find(const char *id)
{
    for (keyword *kw = &keywords[0]; kw->id != NULL; kw++) {
        if (strcasecmp(id, kw->id) == 0) {
            return kw;
        }
    }
    
    return NULL;
}

