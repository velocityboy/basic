#include <math.h>
#include <stdio.h>
#include <strings.h>

#include "builtins.h"
#include "output.h"
#include "runtime.h"
#include "value.h"

const int MAX_ARGS = 16;

typedef struct builtin builtin;

struct builtin
{
    const char *name;
    value *(*execute)(runtime *rt, value **argv);
    int args;
    valuetype types[MAX_ARGS];
};

static value *builtin_abs(runtime *rt, value **argv);
static value *builtin_cos(runtime *rt, value **argv);
static value *builtin_ln(runtime *rt, value **argv);
static value *builtin_log(runtime *rt, value **argv);
static value *builtin_sin(runtime *rt, value **argv);
static value *builtin_tab(runtime *rt, value **argv);
static value *builtin_tan(runtime *rt, value **argv);

static builtin builtins[] =
{
    { "ABS", &builtin_abs, 1, { TYPE_NUMBER } },
    { "COS", &builtin_cos, 1, { TYPE_NUMBER } },
    { "LN", &builtin_ln, 1, { TYPE_NUMBER } },
    { "LOG", &builtin_log, 1, { TYPE_NUMBER } },
    { "SIN", &builtin_sin, 1, { TYPE_NUMBER } },
    { "TAB", &builtin_tab, 1, { TYPE_NUMBER } },
    { "TAN", &builtin_tan, 1, { TYPE_NUMBER } },

    { NULL, NULL, 0, {}}
};

/* Execute a built-in function
 */
value *builtin_execute(runtime *rt, const char *id, int argc, value **argv)
{
    int i = 0;
    
    for (i = 0; builtins[i].name != NULL; i++) {
        if (strcasecmp(builtins[i].name, id) == 0) {
            break;
        }
    }
    
    if (builtins[i].name == NULL) {
        runtime_set_error(rt, "FUNCTION %s IS NOT DEFINED", id);
        return NULL;
    }
    
    return builtins[i].execute(rt, argv);
}

/* Absolute value
 */
value *builtin_abs(runtime *rt, value **argv)
{
    return value_alloc_number(fabs(argv[0]->number));
}

/* Cosine
 */
value *builtin_cos(runtime *rt, value **argv)
{
    return value_alloc_number(cos(argv[0]->number));
}

/* Log base e
 */
value *builtin_ln(runtime *rt, value **argv)
{
    return value_alloc_number(log(argv[0]->number));
}

/* Log base 10
 */
value *builtin_log(runtime *rt, value **argv)
{
    return value_alloc_number(log10(argv[0]->number));
}

/* Sine
 */
value *builtin_sin(runtime *rt, value **argv)
{
    return value_alloc_number(sin(argv[0]->number));
}

/* tab to given position
 */
value *builtin_tab(runtime *rt, value **argv)
{
    int col = (int)argv[0]->number;
    output *out = runtime_get_output(rt);
    output_tab_to_col(out, col);
    return value_alloc_void();
}


/* Tangent
 */
value *builtin_tan(runtime *rt, value **argv)
{
    return value_alloc_number(tan(argv[0]->number));
}
