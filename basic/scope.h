#ifndef scope_h
#define scope_h

/* while BASIC doesn't have scopes in the full sense of a modern language,
 * where it implies a namespace and object lifetime, it does have a function
 * stack (via GOSUB) and loops (FOR/NEXT).
 */

typedef struct scope scope;
typedef struct scope_stack scope_stack;
typedef enum scope_type scope_type;

enum scope_type
{
    SCOPE_GOSUB,
    SCOPE_FOR,
};

struct scope
{
    scope_type type;
    scope *prev;
    
    void (*free)(scope *scp);
};

struct scope_stack
{
    scope *top;
};

extern scope_stack *scope_stack_alloc();
extern void scope_stack_free(scope_stack *stk);
extern void scope_stack_push(scope_stack *stk, scope *scp);
extern void scope_stack_pop(scope_stack *stk);
extern void scope_stack_pop_until(scope_stack *stk, scope_type type);
extern void scope_stack_clear(scope_stack *stk);

#endif /* scope_h */
