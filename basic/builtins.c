#include <math.h>
#include <stdio.h>
#include <strings.h>

#include "builtins.h"
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

static builtin builtins[] =
{
    { "ABS", &builtin_abs, 1, { TYPE_NUMBER } },

    { NULL, NULL, 0, {}}
};

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

value *builtin_abs(runtime *rt, value **argv)
{
    return value_alloc_number(fabs(argv[0]->number));
}
