#ifndef value_h
#define value_h

typedef enum valuealloc valuealloc;
typedef enum valuetype valuetype;
typedef struct value value;

enum valuetype
{
    TYPE_VOID,
    TYPE_NUMBER,
    TYPE_BOOLEAN,
    TYPE_STRING
};

struct value
{
    valuetype type;
    double number;
    int boolean;
    char *string;
};

enum valuealloc {
    VAL_ALLOCATED,
    VAL_COPY,
};

extern value *value_alloc_void();
extern value *value_alloc_number(double v);
extern value *value_alloc_boolean(int v);
extern value *value_alloc_string(char *s, valuealloc allocated);
extern const char *value_describe_type(valuetype type);
extern void value_free(value *v);
extern value *value_clone(value *v);

#endif /* value_h */
