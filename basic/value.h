#ifndef value_h
#define value_h

typedef enum valuetype valuetype;
typedef struct value value;

enum valuetype
{
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

extern value *value_alloc_number(double v);
extern value *value_alloc_boolean(int v);
extern value *value_alloc_string(char *s, int allocated);
extern const char *value_describe_type(valuetype type);
extern void value_free(value *v);

#endif /* value_h */
