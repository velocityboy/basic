#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "output.h"
#include "program.h"
#include "runtime.h"
#include "safemem.h"
#include "scope.h"
#include "statement.h"
#include "value.h"

const int VARCOUNT = 26 * 27;

struct runtime
{
    program *pgm;
    output *out;
    statement *curr_statement;
    value *vars[2 * VARCOUNT];
    statement **statements;
    statement *goto_statement;
    int allocated_statements;
    int used_statements;
    scope_stack *scopes;
    char *error;
};

static int var_is_string(int varidx)
{
    return (varidx >= VARCOUNT) && (varidx < 2 * VARCOUNT);
}

static int var_ref(const char *var);
static void build_statement_index(runtime *rt, program *pgm);

/* Allocate a runtime environment
 */
runtime *runtime_alloc(program *pgm)
{
    runtime *rt = calloc(1, sizeof(runtime));
    rt->pgm = pgm;
    rt->out = output_alloc();
    rt->scopes = scope_stack_alloc();
    return rt;
}

/* Free a runtime environment
 */
void runtime_free(runtime *rt)
{
    if (rt) {
        output_free(rt->out);
        scope_stack_free(rt->scopes);
    }
    free(rt);
}

/* Return the program
 */
program *runtime_get_program(runtime *rt)
{
    return rt->pgm;
}

/* Return the output stream 
 */
output *runtime_get_output(runtime *rt)
{
    return rt->out;
}

/* Run the program
 */
void runtime_run(runtime *rt)
{
    free(rt->error);
    rt->error = NULL;
    rt->goto_statement = NULL;
    
    build_statement_index(rt, rt->pgm);
    scope_stack_clear(rt->scopes);
    
    rt->curr_statement = rt->pgm->head;
    
    while (rt->curr_statement)
    {
        statement *stmt = rt->curr_statement;
        
        rt->goto_statement = NULL;
        
        if (!runtime_execute_statement(rt, stmt)) {
            break;
        }
        
        if (rt->goto_statement) {
            rt->curr_statement = rt->goto_statement;
        } else {
            rt->curr_statement = stmt->next;
        }
    }
}

/* Execute one statement, possibly printing a runtime error
 * Returns 1 on success, else 0
 */
int runtime_execute_statement(runtime *rt, statement *stmt)
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
        
        return 0;
    }
    
    return 1;
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

/* Sets the next statement to execute when the current statment
 * finishes. Sets a runtime error if the target line number doesn't
 * exist.
 */
void runtime_goto(runtime *rt, int line_no)
{
    int low = 0;
    int high = rt->used_statements - 1;
    
    while (low <= high) {
        int m = (low + high) / 2;
        int line_at_m = rt->statements[m]->line;
        
        if (line_at_m < line_no) {
            low = m + 1;
        } else if (line_at_m > line_no) {
            high = m - 1;
        } else {
            rt->goto_statement = rt->statements[m];
            return;
        }
    }
    
    runtime_set_error(rt, "LINE NUMBER %d DOES NOT EXIST", line_no);
}

/* Set the next statement directly
 */
void runtime_set_next_statement(runtime *rt, statement *stmt)
{
    rt->goto_statement = stmt;
}

/* Returns the next statement to be executed. Expected to be called
 * in the context of an executing statement.
 */
statement *runtime_next_statement(runtime *rt)
{
    if (rt->curr_statement) {
        return rt->curr_statement->next;
    }
    return NULL;
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

/* Builds an array version of the statment list. Since we know the 
 * list is in sorted statement order, we can than bsearch it on line
 * number.
 */
void build_statement_index(runtime *rt, program *pgm)
{
    int count = 0;
    
    for (statement *p = pgm->head; p; p = p->next) {
        count++;
    }
    
    if (count > rt->allocated_statements) {
        free(rt->statements);
        rt->statements = safe_calloc(count, sizeof(rt->statements[0]));
        rt->allocated_statements = count;
    }
    
    rt->used_statements = count;
    
    count = 0;
    for (statement *p = pgm->head; p; p = p->next) {
        rt->statements[count++] = p;
    }
}

/* Returns the scope stack
 */
scope_stack *runtime_scope_stack(runtime *rt)
{
    return rt->scopes;
}
