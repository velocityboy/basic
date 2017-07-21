#include <stdio.h>

#include "assert.h"
#include "expression.h"
#include "parser.h"
#include "print.h"
#include "safemem.h"
#include "statement.h"
#include "value.h"

const int TAB_SIZE = 8;

typedef struct print_node print_node;
typedef struct print_part print_part;
typedef enum print_spacing print_spacing;

enum print_spacing
{
    SPC_NONE,
    SPC_ADJACENT,
    SPC_TAB
};

struct print_part
{
    print_part *next;
    expression *exp;
    print_spacing spacing;
};

struct print_node
{
    statement_body body;
    print_part *parts;
    print_part *parts_tail;
};

static void print_execute(statement_body *body);
static void print_free(statement_body *body);
static print_part *part_alloc(expression *exp, print_spacing spacing);
static void part_free(print_part *part);

/* parse the body of a print statement
 */
void print_parse(parser *prs, statement *stmt)
{
    
    print_node *node = (print_node *)safe_calloc(1, sizeof(print_node));

    while (1) {
        if (prs->token_type == TOK_END) {
            break;
        }
        
        expression *exp = expression_parse(prs);

        print_spacing spacing = SPC_NONE;
        if (prs->token_type == TOK_SEMICOLON) {
            spacing = SPC_ADJACENT;
            parse_next_token(prs);
        } else if (prs->token_type == TOK_COMMA) {
            spacing = SPC_TAB;
            parse_next_token(prs);
        }
        
        print_part *part = part_alloc(exp, spacing);
        
        if (node->parts_tail == NULL) {
            node->parts = part;
        } else {
            node->parts_tail->next = part;
        }
        node->parts_tail = part;
        
        if (spacing == SPC_NONE) {
            break;
        }
    }
            
    node->body.execute = &print_execute;
    node->body.free = &print_free;
    
    stmt->body = &node->body;
}

/* free a print node 
 */
void print_free(statement_body *body)
{
    print_node *node = (print_node *)body;
    
    print_part *curr = node->parts;
    while (curr) {
        print_part *next = curr->next;
        expression_free(curr->exp);
        part_free(curr);
        curr = next;
    }
    
    free(node);
}

/* evaluate the print statement
 */
void print_execute(statement_body *body)
{
    print_node *node = (print_node *)body;
    int col = 0;
    
    for (print_part *p = node->parts; p; p = p->next) {
        value *val = expression_evaluate(p->exp);
        
        // TODO really want (a) a more specific reason why expressions fail, and
        // (b) a way to pass this up so the caller can print context like line #
        if (val == NULL) {
            return;
        }
        
        switch (val->type) {
        case TYPE_BOOLEAN:
            col += printf("%d", val->boolean);
            break;
        
        case TYPE_NUMBER:
            col += printf("%lf", val->number);
            break;
        
        case TYPE_STRING:
            col += printf("%s", val->string);
            break;
        }
        
        if (p->spacing == SPC_TAB) {
            int target = ((col + 7) / TAB_SIZE) * TAB_SIZE;
            if (target == col) {
                target += TAB_SIZE;
            }
            target -= col;
            while (target--) {
                putchar(' ');
            }
        }
    }
    
    putchar('\n');
}

/* allocate a part
 */
print_part *part_alloc(expression *exp, print_spacing spacing)
{
    print_part *part = safe_calloc(1, sizeof(print_part));
    part->exp = exp;
    part->spacing = spacing;
    return part;
}

/* free a part
 */
void part_free(print_part *part)
{
    expression_free(part->exp);
    free(part);
}
