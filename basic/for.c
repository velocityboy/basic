#include <string.h>

#include "expression.h"
#include "for.h"
#include "parser.h"
#include "runtime.h"
#include "safemem.h"
#include "scope.h"
#include "statement.h"
#include "stringutil.h"
#include "value.h"

typedef struct for_node for_node;
typedef struct for_scope for_scope;
typedef struct next_node next_node;

static void for_free(statement_body *body);
static void for_execute(statement_body *body, runtime *rt);
static void for_scope_free(scope *scope);
static void next_free(statement_body *body);
static void next_execute(statement_body *body, runtime *rt);

struct for_node
{
    statement_body body;
    char *id;
    expression *start;
    expression *limit;
    expression *step;
};

struct next_node
{
    statement_body body;
    char *id;
};

struct for_scope
{
    scope scope;
    
    statement *loop_top;
    
    /* note that since these never change, ownership remains with
     * the for_node and we should NOT free them when the scope exits.
     */
    char *id;
    value *limit;
    value *step;
};

/* Parse a FOR statement
 */
void for_parse(parser *prs, statement *stmt)
{
    for_node *forn = safe_calloc(1, sizeof(for_node));
    
    if ((forn->id = parser_expect_var(prs)) == NULL ||
        !parser_expect_operator(prs, TOK_EQUALS) ||
        (forn->start = expression_parse(prs)) == NULL ||
        !parser_expect_id(prs, "TO") ||
        (forn->limit = expression_parse(prs)) == NULL) {
        for_free(&forn->body);
        return;
    }
    
    if (prs->token_type != TOK_END) {
        if (!parser_expect_id(prs, "STEP") ||
            (forn->step = expression_parse(prs)) == NULL ||
            !parser_expect_end_of_line(prs)) {
            for_free(&forn->body);
            return;
        }
    }
    
    if (forn->id[strlen(forn->id) - 1] == '$') {
        parser_set_error(prs, "FOR LOOP INDEX MAY NOT BE STRING");
        for_free(&forn->body);
        return;
    }
    
    strupr(forn->id);
    
    forn->body.execute = &for_execute;
    forn->body.free = &for_free;
    stmt->body = &forn->body;
}

/* Parse a NEXT statement
 */
void next_parse(parser *prs, statement *stmt)
{
    next_node *next = safe_calloc(1, sizeof(next_node));
    
    if (prs->token_type != TOK_END) {
        if ((next->id = parser_expect_var(prs)) == NULL ||
            !parser_expect_end_of_line(prs)) {
            next_free(&next->body);
            return;
        }
    }
    
    if (next->id) {
        strupr(next->id);
    }
    
    next->body.execute = next_execute;
    next->body.free = next_free;
    
    stmt->body = &next->body;
}

/* Free a FOR node
 */
void for_free(statement_body *body)
{
    for_node *forn = (for_node *)body;
    
    if (forn) {
        free(forn->id);
        expression_free(forn->start);
        expression_free(forn->limit);
        expression_free(forn->step);
    }

    free(body);
}

/* Free a NEXT node 
 */
void next_free(statement_body *body)
{
    next_node *next = (next_node *)body;
    
    if (next) {
        free(next->id);
    }
    
    free(body);
}

/* Free a for scope
 */
void for_scope_free(scope *scp)
{
    free(scp);
}

/* Execute for
 */
void for_execute(statement_body *body, runtime *rt)
{
    for_node *forn = (for_node *)body;
    for_scope *scp = safe_calloc(1, sizeof(for_scope));
    
    scp->scope.free = &for_scope_free;
    scp->scope.type = SCOPE_FOR;
    scp->id = forn->id;
    
    /* NOTE that we lock the loop iteration parameters the first time through
     * the loop. This means that if the body of the loop changes the values
     * of those expressions, the loop won't change.
     * 
     * 1000 LET N = 10
     * 1010 FOR I = 1 TO N
     * 1020 LET N = 20
     * 1030 NEXT I
     * 
     * only executes 10 times.
     *
     * Note that the code *can* change the loop index variable (I in this case)
     */
    scp->limit = expression_evaluate(forn->limit, rt);
    scp->step = forn->step ? expression_evaluate(forn->step, rt) : value_alloc_number(1);
    scp->loop_top = runtime_next_statement(rt);
    
    runtime_setvar(rt, scp->id, expression_evaluate(forn->start, rt));

    scope_stack_push(runtime_scope_stack(rt), &scp->scope);
}

/* Execute next
 */
void next_execute(statement_body *body, runtime *rt)
{
    next_node *next = (next_node *)body;
    scope_stack *stk = runtime_scope_stack(rt);
    
    if (stk->top == NULL || stk->top->type != SCOPE_FOR) {
        runtime_set_error(rt, "NESTING ERROR");
        return;
    }
    
    for_scope *scp = (for_scope *)stk->top;
    
    if (next->id != NULL && strcasecmp(scp->id, next->id) != 0) {
        runtime_set_error(rt, "NEXT INDEX %s DOES NOT MATCH FOR INDEX %s", next->id, scp->id);
        return;
    }
    
    value *index = runtime_getvar(rt, scp->id);
    if (index->type != TYPE_NUMBER) {
        /* this should be impossible */
        runtime_set_error(rt, "INDEX VARIABLE %s IS NO LONGER A NUMBER", scp->id);
        return;
    }
    
    double delta = scp->step->number;
    index->number += delta;
    
    int done = 0;
    if (delta < 0) {
        done = index->number < scp->limit->number;
    } else {
        done = index->number > scp->limit->number;
    }
    
    if (!done) {
        runtime_set_next_statement(rt, scp->loop_top);
    } else {
        scope_stack_pop(stk);
    }
}
