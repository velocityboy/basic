#include "expression.h"
#include "list.h"
#include "parser.h"
#include "program.h"
#include "runtime.h"
#include "safemem.h"
#include "statement.h"
#include "value.h"

typedef struct list_node list_node;

struct list_node
{
    statement_body body;
    int first;
    int last;
};

static void list_execute(statement_body *body, runtime *rt);
static void list_free(statement_body *body);

/* Parse the list statement
 */
void list_parse(parser *prs, statement *stmt)
{
    list_node *lst = safe_calloc(1, sizeof(list_node));
    lst->first = 0;
    lst->last = -1;
    
    if (prs->token_type != TOK_END) {
        if ((lst->first = parser_expect_line_no(prs, 1)) == -1) {
            free(&lst->body);
            return;
        }
        
        if (prs->token_type != TOK_END) {
            if (!parser_expect_operator(prs, TOK_MINUS) ||
                (lst->last = parser_expect_line_no(prs, 1)) == -1) {
                free(&lst->body);
                return;
            }
        }
        
        if (!parser_expect_end_of_line(prs)) {
            free(&lst->body);
            return;
        }
    }
    
    lst->body.free = &list_free;
    lst->body.execute = &list_execute;

    stmt->body = &lst->body;
}

/* execute a list node
 */
void list_execute(statement_body *body, runtime *rt)
{
    list_node *lst = (list_node *)body;
    
    program *pgm = runtime_get_program(rt);
    for (statement *stmt = pgm->head; stmt; stmt = stmt->next) {
        if (stmt->line < lst->first) {
            continue;
        }
        
        if (lst->last != -1 && stmt->line > lst->last) {
            break;
        }
        
        printf("%s\n", stmt->text);
    }
}

/* free a list node
 */
void list_free(statement_body *body)
{
    free(body);
}


