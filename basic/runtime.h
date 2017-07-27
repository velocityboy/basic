#ifndef runtime_h
#define runtime_h

typedef struct output output;
typedef struct program program;
typedef struct runtime runtime;
typedef struct scope scope;
typedef struct scope_stack scope_stack;
typedef struct statement statement;
typedef struct value value;

extern runtime *runtime_alloc(program *pgm);
extern void runtime_free(runtime *rt);
extern program *runtime_get_program(runtime *rt);
extern output *runtime_get_output(runtime *rt);
extern void runtime_run(runtime *rt);
extern int runtime_execute_statement(runtime *rt, statement *stmt);
extern void runtime_set_error(runtime *rt, const char *fmt, ...);
extern value *runtime_getvar(runtime *rt, const char *var);
extern int runtime_setvar(runtime *rt, const char *var, value *value);
extern void runtime_goto(runtime *rt, int line_no);
extern void runtime_set_next_statement(runtime *rt, statement *stmt);
extern statement *runtime_next_statement(runtime *rt);
extern scope_stack *runtime_scope_stack(runtime *rt);

#endif /* runtime_h */
