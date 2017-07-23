#include "expression.h"
#include "gosub.h"
#include "parser.h"
#include "runtime.h"
#include "safemem.h"
#include "scope.h"
#include "statement.h"
#include "value.h"

typedef struct gosub_node gosub_node;
typedef struct gosub_scope gosub_scope;
typedef struct return_node return_node;

static void gosub_free(statement_body *body);
static void gosub_execute(statement_body *body, runtime *rt);
static void gosub_scope_free(scope *scope);
static void return_free(statement_body *body);
static void return_execute(statement_body *body, runtime *rt);

struct gosub_node
{
    statement_body body;
    int target;
};

struct return_node
{
    statement_body body;
};

struct gosub_scope
{
    scope scope;
    statement *return_stmt;
};

/* Parse a GOSUB statement
 */
void gosub_parse(parser *prs, statement *stmt)
{
    gosub_node *gsu = safe_calloc(1, sizeof(gosub_node));
    
    if ((gsu->target = parser_expect_line_no(prs, 1)) == -1 ||
        !parser_expect_end_of_line(prs)) {
        gosub_free(&gsu->body);
        return;
    }
    
    gsu->body.free = &gosub_free;
    gsu->body.execute = &gosub_execute;

    stmt->body = &gsu->body;
}

/* Parse a RETURN statement 
 */
void return_parse(parser *prs, statement *stmt)
{
    return_node *rtn = safe_calloc(1, sizeof(return_node));
    
    if (!parser_expect_end_of_line(prs)) {
        return_free(&rtn->body);
        return;
    }
    
    rtn->body.free = &return_free;
    rtn->body.execute = &return_execute;

    stmt->body = &rtn->body;
}

/* Free a GOSUB node
 */
void gosub_free(statement_body *body)
{
    free(body);
}

/* Execute gosub
 */
void gosub_execute(statement_body *body, runtime *rt)
{
    gosub_node *gsu = (gosub_node *)body;
    gosub_scope *scope = safe_calloc(1, sizeof(gosub_scope));
    scope->scope.type = SCOPE_GOSUB;
    scope->scope.free = &gosub_scope_free;
    scope->return_stmt = runtime_next_statement(rt);
    
    scope_stack_push(runtime_scope_stack(rt), &scope->scope);
    runtime_goto(rt, gsu->target);
}

/* Free a gosub scope node
 */
void gosub_scope_free(scope *scope)
{
    free(scope);
}

/* Free a RETURN node
 */
void return_free(statement_body *body)
{
    free(body);
}

/* execute return
 */
void return_execute(statement_body *body, runtime *rt)
{
    scope_stack *stk = runtime_scope_stack(rt);
    
    /* we pop until we find a return instead of just looking at the top
     * because we want to be able to return out of loops like:
     *
     * 1000 FOR I = 1 TO 100
     * 1010   IF I <> 500 THEN 1030
     * 1020   RETURN
     * 1030 NEXT I
     *
     */
    scope_stack_pop_until(stk, SCOPE_GOSUB);
    if (stk->top == NULL) {
        runtime_set_error(rt, "RETURN WITHOUT GOSUB");
        return;
    }
    
    gosub_scope *scp = (gosub_scope *)stk->top;
    
    runtime_set_next_statement(rt, scp->return_stmt);
    scope_stack_pop(stk);
}

