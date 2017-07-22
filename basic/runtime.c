#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "program.h"
#include "runtime.h"
#include "safemem.h"
#include "statement.h"
#include "value.h"

const int VARCOUNT = 26 * 27;

struct runtime
{
    value *vars[2 * VARCOUNT];
    char *error;
};

static int var_is_string(int varidx)
{
    return (varidx >= VARCOUNT) && (varidx < 2 * VARCOUNT);
}

static int var_ref(const char *var);


/* Allocate a runtime environment
 */
runtime *runtime_alloc()
{
    runtime *rt = calloc(1, sizeof(runtime));
    return rt;
}

/* Free a runtime environment
 */
void runtime_free(runtime *rt)
{
    free(rt);
}

/* Run a program
 */
void runtime_run(runtime *rt, program *pgm)
{
    free(rt->error);
    rt->error = NULL;
    
    statement *stmt = pgm->head;
    
    while (stmt)
    {
        stmt->body->execute(stmt->body, rt);
        
        if (rt->error) {
            fprintf(stderr, "\n%s", rt->error);
            if (stmt->line >= 0) {
                fprintf(stderr, " IN %d", stmt->line);
            }
            fprintf(stderr, "\n");
            
            free(rt->error);
            rt->error = NULL;
            break;
        }
        
        stmt = stmt->next;
    }
}

/* Set a runtime error, which will cause the program to abort
 * after the current statment
 */
void runtime_set_error(runtime *rt, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);    
    int n = vsnprintf(NULL, 0, fmt, args);
    char *msg = safe_malloc(n + 1);

    va_start(args, fmt);
    vsnprintf(msg, n + 1, fmt, args);
    
    if (rt->error) {
        free(rt->error);
        rt->error = NULL;
    }
    
    rt->error = msg;
    
    va_end(args);
}

/* Get a variable
 * Returns NULL if the variable is undefined or if the name is
 * invalid
 */
value *runtime_getvar(runtime *rt, const char *var)
{
    int varidx = var_ref(var);
    if (varidx < 0) {
        return NULL;
    }
    
    return rt->vars[varidx];
}

/* Set a variable. Returns 0 on failure, 1 on success
 */
int runtime_setvar(runtime *rt, const char *var, value *value)
{
    int varidx = var_ref(var);
    if (varidx < 0) {
        return 0;
    }
    
    if (var_is_string(varidx)) {
        if (value->type != TYPE_STRING) {
            return 0;
        }
    } else if (value->type != TYPE_NUMBER) {
        return 0;
    }
    
    if (rt->vars[varidx] != NULL) {
        value_free(rt->vars[varidx]);
    }
    rt->vars[varidx] = value;
    
    return 1;
}

/* Parse a variable reference
 * Returns a variable reference or -1 if invalid
 */
int var_ref(const char *var)
{
    size_t len = strlen(var);
    if (len == 0) {
        /* empty name */
        return -1;
    }
    
    /* if it ends in $, it's in the string set
     */
    int base = 0;
    if (var[len - 1] == '$') {
        if (len == 1) {
            return -1;
        }
        base = VARCOUNT;
        len--;
    }
    
    if (len > 2) {
        /* only two characters of var are significant 
         */
        len = 2;
    }
    
    if (!isalpha(var[0])) {
        return -1;
    }

    if (len == 1) {
        return base + toupper(var[0]) - 'A';
    }
    
    if (!isalpha(var[1])) {
        return -1;
    }

    return base + 26 + 26 * (toupper(var[0] - 'A')) + toupper(var[1] - 'Z');
}

