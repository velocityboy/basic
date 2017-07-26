#include "expression.h"
#include "new.h"
#include "parser.h"
#include "program.h"
#include "runtime.h"
#include "safemem.h"
#include "statement.h"

typedef struct new_node new_node;

struct new_node
{
    statement_body body;
};

static void new_execute(statement_body *body, runtime *rt);
static void new_free(statement_body *body);

/* Parse the new statement
 */
void new_parse(parser *prs, statement *stmt)
{
    new_node *newn = safe_calloc(1, sizeof(new_node));
    newn->body.execute = &new_execute;
    newn->body.free = &new_free;
    stmt->body = &newn->body;
}

/* execute a new node
 */
void new_execute(statement_body *body, runtime *rt)
{
    program *pgm = runtime_get_program(rt);
    program_new(pgm);
}

/* free a new node
 */
void new_free(statement_body *body)
{
    free(body);
}


