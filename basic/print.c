#include "assert.h"
#include "expression.h"
#include "print.h"
#include "safemem.h"
#include "statement.h"

typedef struct print_node print_node;

struct print_node
{
    statement_body body;
    expression *exp;
};

#define NODE(body) ((print_node *)(body))

static void print_execute(statement_body *body);
static void print_free(statement_body *body);

/* parse the body of a print statement
 */
void print_parse(parser *prs, statement *stmt)
{
    expression *exp = expression_parse(prs);
    
    print_node *node = (print_node *)safe_calloc(1, sizeof(print_node));
    node->exp = exp;
    node->body.execute = &print_execute;
    node->body.free = &print_free;
    
    stmt->body = &node->body;
    
    assert(node == (print_node*)&node->body);
}

/* free a print node 
 */
void print_free(statement_body *body)
{
    print_node *node = NODE(body);
    expression_free(node->exp);
    free(node);
}

/* evaluate the print statement
 */
void print_execute(statement_body *body)
{
}
