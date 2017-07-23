#include "safemem.h"
#include "scope.h"

/* Allocate a scope stack
 */
scope_stack *scope_stack_alloc()
{
    return safe_calloc(1, sizeof(scope_stack));
}

/* Free a scope stack
 */
void scope_stack_free(scope_stack *stk)
{
    scope_stack_clear(stk);
    free(stk);
}

/* Push a scope onto the stack. scp must be dynamically allocated
 * and the stack becomes the owner of the memory.
 */
void scope_stack_push(scope_stack *stk, scope *scp)
{
    scp->prev = stk->top;
    stk->top = scp;
}

/* Pop and free the top of the stack, if it's not empty
 */
void scope_stack_pop(scope_stack *stk)
{
    if (stk->top) {
        scope *prev = stk->top->prev;
        stk->top->free(stk->top);
        stk->top = prev;
    }
}

/* pop scopes until a scope of the given type is found.
 * On return, the stack will either by empty, or the new top will be
 * the first scope of the given type.
 */
void scope_stack_pop_until(scope_stack *stk, scope_type type)
{
    while (stk->top && stk->top->type != type) {
        scope *prev = stk->top->prev;
        stk->top->free(stk->top);
        stk->top = prev;
    }
}


/* Clear all elements from the stack
 */
void scope_stack_clear(scope_stack *stk)
{
    while (stk->top) {
        scope *prev = stk->top->prev;
        stk->top->free(stk->top);
        stk->top = prev;
    }
}
