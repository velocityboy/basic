#include "safemem.h"
#include "value.h"

/* Create a number value
 */
value *value_alloc_number(double v)
{
    value *val = safe_calloc(1, sizeof(value));
    val->type = TYPE_NUMBER;
    val->number = v;
    
    return val;
}

/* Create a boolean value (guaranteed to be zero or one)
 */
value *value_alloc_boolean(int v)
{
    value *val = safe_calloc(1, sizeof(value));
    val->type = TYPE_BOOLEAN;
    val->boolean = v != 0;
    
    return val;
}

/* Create a string value (which owns its own memory)
 */
value *value_alloc_string(const char *s)
{
    value *val = safe_calloc(1, sizeof(value));
    val->type = TYPE_STRING;
    val->string = safe_strdup(s);
    
    return val;
}

/* Free a value
 */
void value_free(value *v)
{
    if (v && v->type == TYPE_STRING) {
        free(v->string);
    }
    free(v);
}
