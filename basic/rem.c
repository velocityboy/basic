#include "expression.h"
#include "let.h"
#include "parser.h"
#include "runtime.h"
#include "safemem.h"
#include "statement.h"

typedef struct rem_node rem_node;

struct rem_node
{
    statement_body body;
};

static void rem_execute(statement_body *body, runtime *rt);
static void rem_free(statement_body *body);

/* Parse the rem statement
 */
void rem_parse(parser *prs, statement *stmt)
{
    rem_node *rem = safe_calloc(1, sizeof(rem_node));
    rem->body.execute = &rem_execute;
    rem->body.free = &rem_free;
    stmt->body = &rem->body;
}

/* execute a rem node
 */
void rem_execute(statement_body *body, runtime *rt)
{
}

/* free a rem node
 */
void rem_free(statement_body *body)
{
    free(body);
}


