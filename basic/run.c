#include "expression.h"
#include "parser.h"
#include "run.h"
#include "runtime.h"
#include "safemem.h"
#include "statement.h"

typedef struct run_node run_node;

struct run_node
{
    statement_body body;
};

static void run_execute(statement_body *body, runtime *rt);
static void run_free(statement_body *body);

/* Parse the run statement
 */
void run_parse(parser *prs, statement *stmt)
{
    run_node *run = safe_calloc(1, sizeof(run_node));
    run->body.execute = &run_execute;
    run->body.free = &run_free;
    stmt->body = &run->body;
}

/* execute a run node
 */
void run_execute(statement_body *body, runtime *rt)
{
    runtime_run(rt);
}

/* free a run node
 */
void run_free(statement_body *body)
{
    free(body);
}


