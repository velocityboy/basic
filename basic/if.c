#include "expression.h"
#include "if.h"
#include "parser.h"
#include "runtime.h"
#include "safemem.h"
#include "statement.h"
#include "value.h"

typedef struct if_node if_node;

struct if_node
{
    statement_body body;
    expression *exp;
    int then_target;
    int else_target;
};

static void if_execute(statement_body *body, runtime *rt);
static void if_free(statement_body *body);

/* Parse the if statement
 */
void if_parse(parser *prs, statement *stmt)
{
    if_node *ifn = safe_calloc(1, sizeof(if_node));
    
    if ((ifn->exp = expression_parse(prs)) == NULL ||
        !parser_expect_id(prs, "THEN") ||
        (ifn->then_target = parser_expect_line_no(prs, 0)) == -1) {
        if_free(&ifn->body);
        return;
    }
    
    if (prs->token_type != TOK_END) {
        if (!parser_expect_id(prs, "ELSE") ||
            (ifn->else_target = parser_expect_line_no(prs, 0)) == -1) {
            if_free(&ifn->body);
            return;
        }
    }
    
    if (!parser_expect_end_of_line(prs)) {
        if_free(&ifn->body);
        return;
    }
    
    ifn->body.free = &if_free;
    ifn->body.execute = &if_execute;

    stmt->body = &ifn->body;
}

/* execute an if node
 */
void if_execute(statement_body *body, runtime *rt)
{
    if_node *ifn = (if_node*)body;
    
    value *v = expression_evaluate(ifn->exp, rt);
    if (v == NULL || v->type != TYPE_BOOLEAN) {
        runtime_set_error(rt, "IF EXPRESSION NOT COMPARISON");
    } else if (v->boolean) {
        runtime_goto(rt, ifn->then_target);
    } else if (ifn->else_target != -1) {
        runtime_goto(rt, ifn->else_target);
    }
    
    value_free(v);
}

/* free an if node
 */
void if_free(statement_body *body)
{
    if_node *ifn = (if_node *)body;
    
    if (ifn) {
        expression_free(ifn->exp);
    }

    free(ifn);
}


