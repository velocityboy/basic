#include "expression.h"
#include "goto.h"
#include "parser.h"
#include "runtime.h"
#include "safemem.h"
#include "statement.h"
#include "value.h"

typedef struct goto_node goto_node;

struct goto_node
{
    statement_body body;
    int target;
};

static void goto_execute(statement_body *body, runtime *rt);
static void goto_free(statement_body *body);

/* Parse the goto statement
 */
void goto_parse(parser *prs, statement *stmt)
{
    goto_node *gto = safe_calloc(1, sizeof(goto_node));
    
    if ((gto->target = parser_expect_line_no(prs, 1)) == -1 ||
        !parser_expect_end_of_line(prs)) {
        goto_free(&gto->body);
        return;
    }
    
    gto->body.free = &goto_free;
    gto->body.execute = &goto_execute;

    stmt->body = &gto->body;
}

/* execute a goto node
 */
void goto_execute(statement_body *body, runtime *rt)
{
    goto_node *gto = (goto_node*)body;
    runtime_goto(rt, gto->target);
}

/* free a goto node
 */
void goto_free(statement_body *body)
{
    free(body);
}


