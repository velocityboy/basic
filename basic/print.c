#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "assert.h"
#include "expression.h"
#include "output.h"
#include "parser.h"
#include "print.h"
#include "runtime.h"
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

static void print_execute(statement_body *body, runtime *rt);
static void print_number(output *out, const char *fmt, double number);
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
        if (exp == NULL) {
            break;
        }
        
        print_spacing spacing = SPC_NONE;
        if (prs->token_type == TOK_SEMICOLON) {
            spacing = SPC_ADJACENT;
            parse_next_token(prs);
        } else if (prs->token_type == TOK_COMMA) {
            spacing = SPC_TAB;
            parse_next_token(prs);
        } else if (prs->token_type != TOK_END) {
            parser_set_error(prs, "EXPECTED , or ; BETWEEN PRINT ITEMS");            
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
    if (node == NULL) {
        return;
    }
    
    print_part *curr = node->parts;
    while (curr) {
        print_part *next = curr->next;
        part_free(curr);
        curr = next;
    }
    
    free(node);
}

/* evaluate the print statement
 */
void print_execute(statement_body *body, runtime *rt)
{
    output *out = runtime_get_output(rt);
    print_node *node = (print_node *)body;
    
    
    for (print_part *p = node->parts; p; p = p->next) {
        value *val = expression_evaluate(p->exp, rt);
        
        // TODO really want (a) a more specific reason why expressions fail, and
        // (b) a way to pass this up so the caller can print context like line #
        if (val == NULL) {
            return;
        }
        
        switch (val->type) {
        case TYPE_VOID:
            break;
            
        case TYPE_BOOLEAN:
            output_print(out, "%d", val->boolean);
            break;
        
        case TYPE_NUMBER:
            print_number(out, "%lf", val->number);
            break;
        
        case TYPE_STRING:
            output_print(out, "%s", val->string);
            break;
        }
        
        if (p->spacing == SPC_TAB) {
            output_print(out, "\t");
        }
    }
    
    output_print(out, "\n");
}

/* Format and print a number. Mostly we want to get rid of
 * trailing zeroes past the decimal (1.20000 should be 1.2)
 * which printf format strings don't reresent.
 */
void print_number(output *out, const char *fmt, double number)
{
    char str[32];
    char *strp = str;
    
    int len = snprintf(str, sizeof(str), fmt, number);
    if (len > sizeof(str)) {
        strp = safe_malloc(len + 1);
        snprintf(strp, len + 1, fmt, number);
    }
    
    char *p = strp + len - 1;
    
    while (p >= strp && *p == '0') {
        p--;
    }
    p++;
    
    if (p < strp + len) {
        /* p now points at the first of a trailing string of
         * zeroes. make sure we're actually looking at digits
         * past a decimal point (we could have exponential 
         * notation and something like 1.0E+10)
         */
        char *q = p;
        
        while (q >= strp) {
            if (!isdigit(*q)) {
                break;
            }
            q--;
        }
        
        if (q >= strp && *q == '.') {
            if (p == q + 1) {
                /* there are no non-zero digits to the right of the decimal
                 * point, so remove that as well.
                 */
                p--;
            }
            
            *p = '\0';
        }
    }
    
    output_print(out, "%s", strp);
    
    if (strp != str) {
        free(strp);
    }
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
    if (part) {
        expression_free(part->exp);
    }
    free(part);
}
