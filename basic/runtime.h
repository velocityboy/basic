#ifndef runtime_h
#define runtime_h

typedef struct program program;
typedef struct runtime runtime;
typedef struct value value;

extern runtime *runtime_alloc();
extern void runtime_free(runtime *rt);
extern void runtime_run(runtime *rt, program *pgm);
extern void runtime_set_error(runtime *rt, const char *fmt, ...);
extern value *runtime_getvar(runtime *rt, const char *var);
extern int runtime_setvar(runtime *rt, const char *var, value *value);
extern void runtime_goto(runtime *rt, int line_no);


#endif /* runtime_h */
