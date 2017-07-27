#include "expression.h"
#include "let.h"
#include "parser.h"
#include "runtime.h"
#include "safemem.h"
#include "statement.h"

typedef struct let_node let_node;

struct let_node
{
    statement_body body;
    char *id;
    expression *exp;
};

static void let_execute(statement_body *body, runtime *rt);
static void let_free(statement_body *body);

/* Parse the let statement
 */
void let_parse(parser *prs, statement *stmt)
{
    let_node *let = safe_calloc(1, sizeof(let_node));
    
    if (prs->token_type != TOK_IDENTIFIER) {
        let_free(&let->body);
        parser_set_error(prs, "IDENTIFIER EXPECTED");
        return;
    }
    
    let->id = parser_extract_token_text(prs);
    
    parse_next_token(prs);
    
    if (prs->token_type != TOK_EQUALS) {
        let_free(&let->body);
        parser_set_error(prs, "EQUALS EXPECTED");
        return;
    }
    
    parse_next_token(prs);
    
    let->exp = expression_parse(prs);
    if (let->exp == NULL) {
        let_free(&let->body);
        return;
    }
    
    let->body.execute = &let_execute;
    let->body.free = &let_free;
    
    stmt->body = &let->body;
    
}

/* execute a let node
 */
void let_execute(statement_body *body, runtime *rt)
{
    let_node *let = (let_node *)body;
    value *val = expression_evaluate(let->exp, rt);
    
    if (val) {
        runtime_setvar(rt, let->id, val);
    }
}

/* free a let node
 */
void let_free(statement_body *body)
{
    let_node *let = (let_node *)body;
    
    if (let) {
        free(let->id);
        expression_free(let->exp);
    }

    free(let);
}


