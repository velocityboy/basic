#include <stdio.h>
#include <string.h>

#include "keyword.h"
#include "print.h"
#include "safemem.h"
#include "stringutil.h"

#define MAX_ID 10

static keyword keywords[] =
{
    { "PRINT",  KWFL_OK_IN_STMT | KWFL_OK_IN_REPL, &print_parse },

    { NULL, 0 }
};

/* Find a keyword in the table. Returns a pointer to the keyword
 * or NULL if there is no match.
 */
keyword *kw_find(const char *id)
{
    char upper_id[MAX_ID];
    
    strncpy(upper_id, id, MAX_ID - 1);
    upper_id[MAX_ID - 1] = '\0';
    strupper(upper_id);
    
    for (keyword *kw = &keywords[0]; kw->id != NULL; kw++) {
        if (strcmp(upper_id, kw->id) == 0) {
            return kw;
        }
    }
    
    return NULL;
}

