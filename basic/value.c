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
 * if allocated is set then the caller has already allocated the string
 * and the value should take ownership
 */
value *value_alloc_string(char *s, valuealloc allocated)
{
    value *val = safe_calloc(1, sizeof(value));
    val->type = TYPE_STRING;
    if (allocated == VAL_ALLOCATED) {
        val->string = s;
    } else {
        val->string = safe_strdup(s);
    }
    
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

/* Return a static description of a value type
 */
const char *value_describe_type(valuetype type)
{
    switch (type) {
        case TYPE_NUMBER: return "NUMBER";
        case TYPE_STRING: return "STRING";
        case TYPE_BOOLEAN: return "BOOLEAN";
    }
    
    return "UNKNOWN";
}

/* Clone a value
 */
value *value_clone(value *v)
{
    value *new_val = safe_calloc(1, sizeof(value));
    *new_val = *v;
  
    if (new_val->type == TYPE_STRING) {
        new_val->string = safe_strdup(new_val->string);
    }
  
    return new_val;
}
