#ifndef builtins_h
#define builtins_h

typedef struct runtime runtime;
typedef struct value value;

extern value *builtin_execute(runtime *rt, const char *id, int argc, value **argv);

#endif /* builtins_h */
