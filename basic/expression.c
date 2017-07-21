#include <assert.h>
#include <string.h>

#include "expression.h"
#include "parser.h"
#include "safemem.h"
#include "stringutil.h"
#include "value.h"

typedef struct binop binop;
typedef struct litop litop;
typedef struct unop unop;
typedef struct varref varref;

struct expression
{
    expopnode *root;
};

struct binop
{
    expopnode opnode;
    expopnode *left;
    expopnode *right;
};

struct litop
{
    expopnode opnode;
    value *literal;
};

struct unop
{
    expopnode opnode;
    expopnode *value;
};

struct varref
{
    expopnode opnode;
    char *varname;
};

static value *eval_less(expopnode *node);
static value *eval_unary_minus(expopnode *node);
static expopnode *parse_expression(parser *prs);
static expopnode *parse_relop_term(parser *prs);
static expopnode *parse_sum_term(parser *prs);
static expopnode *parse_mul_term(parser *prs);
static expopnode *parse_unary_term(parser *prs);
static expopnode *parse_paren_term(parser *prs);

static void free_binop(expopnode *node);
static expopnode *alloc_binop(token_type op, expopnode *left, expopnode *right);
static void free_unop(expopnode *node);
static expopnode *alloc_unop(token_type op, expopnode *value);

static value *eval_literal(expopnode *node);
static void free_litop(expopnode *node);
static expopnode *alloc_literal(value *value);

static value *eval_varref(expopnode *node);
static void free_varref(expopnode *node);
static expopnode *alloc_varref(char *varname);


/* top level expression parser
 *
 * precedence
 * highest  A  *|/  B
 *          A  +|-  B
 *          A relop B
 */
expression *expression_parse(parser *prs)
{
    expression *exp = safe_calloc(1, sizeof(expression));
    
    exp->root = parse_expression(prs);
    if (exp->root == NULL) {
        free(exp);
        exp = NULL;
    }
    
    return exp;
}

/* Free an expression tree
 */
void expression_free(expression *exp)
{
    if (exp->root) {
        exp->root->free(exp->root);
    }
    free(exp);
}

/* Evaluate an expression
 */
value *expression_evaluate(expression *exp)
{
    return exp->root->evaluate(exp->root);
}


/* Parse top level of expression
 */
expopnode *parse_expression(parser *prs)
{
    expopnode *left = parse_relop_term(prs);
    if (left == NULL) {
        return NULL;
    }
    
    /* note that unlike some other languages, we don't allow chaining
     * of relational operators
     */
     
    if (is_relop(prs->token_type)) {
        token_type op = prs->token_type;
        
        parse_next_token(prs);
        expopnode *right = parse_relop_term(prs);
        if (right == NULL) {
            left->free(left);
            return NULL;
        }
        
        left = alloc_binop(op, left, right);
    }
    
    return left;
}

/* Parse the operand of a relational operator
 */
expopnode *parse_relop_term(parser *prs)
{
    expopnode *left = parse_sum_term(prs);
    if (left == NULL) {
        return NULL;
    }
    
    while (prs->token_type == TOK_PLUS || prs->token_type == TOK_MINUS) {
        token_type op = prs->token_type;
        parse_next_token(prs);
        
        expopnode *right = parse_sum_term(prs);
        if (right == NULL) {
            left->free(left);
            return NULL;
        }
        
        left = alloc_binop(op, left, right);
    }
    
    return left;
}

/* Parse the operand of a sum operator
 */
expopnode *parse_sum_term(parser *prs)
{
    expopnode *left = parse_mul_term(prs);

    if (left == NULL) {
        return NULL;
    }
    
    while (prs->token_type == TOK_TIMES || prs->token_type == TOK_DIVIDE) {
        token_type op = prs->token_type;
        parse_next_token(prs);
        
        expopnode *right = parse_sum_term(prs);
        if (right == NULL) {
            left->free(left);
            return NULL;
        }
        left = alloc_binop(op, left, right);
    }
    
    return left;
}

/* Parse the operand of a mul operator
 */
expopnode *parse_mul_term(parser *prs)
{
    int negate = 0;
    
    if (prs->token_type == TOK_MINUS) {
        ++negate;
        parse_next_token(prs);
    }
    
    expopnode *value = parse_unary_term(prs);
    
    if (value && negate) {
        value = alloc_unop(prs->token_type, value);
    }
    
    return value;
}

/* Parse the operand of unary negation
 */
expopnode *parse_unary_term(parser *prs)
{
    if (prs->token_type == TOK_LPAREN) {
        parse_next_token(prs);
        
        expopnode *ret = parse_expression(prs);
        if (!ret) {
            return NULL;
        }
        
        if (prs->token_type != TOK_RPAREN) {
            parser_set_error(prs, "UNBALANCED PARENTHESES");
            ret->free(ret);
            return NULL;
        }
        parse_next_token(prs);
        
        return ret;
    }
    
    return parse_paren_term(prs);
}

/* Parse a value
 */
expopnode *parse_paren_term(parser *prs)
{
    expopnode *ret = NULL;
    char *text = NULL;
    double num = 0;
    
    switch (prs->token_type) {
    case TOK_STRING:
        text = parser_extract_token_text(prs);
        strunquote(text);
        ret = alloc_literal(value_alloc_string(text));
        free(text);
        parse_next_token(prs);
        break;
        
    case TOK_NUMBER:
        text = parser_extract_token_text(prs);
        sscanf(text, "%lf", &num);
        ret = alloc_literal(value_alloc_number(num));
        free(text);
        parse_next_token(prs);
        break;
        
    case TOK_IDENTIFIER:
        text = parser_extract_token_text(prs);
        ret = alloc_varref(text);
        /* do not free(text) - varref owns it now */
        parse_next_token(prs);
        break;
    
    default:
        text = parser_describe_token(prs);
        parser_set_error(prs, "%s FOUND, LITERAL OR VARIABLE EXPECTED", text);
        free(text);
        break;
    }

    return ret;
}

/* runtime for < operator
 */
value *eval_less(expopnode *node)
{
    binop *bop = (binop *)node;
    value *left = bop->left->evaluate(bop->left);
    value *right = bop->right->evaluate(bop->right);
    
    value *ret = NULL;
    
    if (left == NULL || right == NULL) {
    } else if (left->type == TYPE_STRING && right->type == TYPE_STRING) {
        ret = value_alloc_boolean(strcmp(left->string, right->string) < 0);
    } else if (left->type == TYPE_NUMBER && right->type == TYPE_STRING) {
        ret = value_alloc_boolean(left->number < right->number);
    }
    
    return ret;
}

/* runtime for unary minus
 */
value *eval_unary_minus(expopnode *node)
{
    unop *uop = (unop *)node;
    
    value *ret = NULL;
    
    value *val = uop->value->evaluate(uop->value);
    
    if (val) {
        if (val->type == TOK_NUMBER) {
            ret = value_alloc_number(-val->number);
        }
    }
    
    return ret;
}


/* free a binary operator
 */
void free_binop(expopnode *node)
{
    binop *bop = (binop *)node;
    
    if (bop->left) {
        bop->left->free(bop->left);
    }
    
    if (bop->right) {
        bop->right->free(bop->right);
    }
    
    free(node);
}

/* Allocate a binary operator node
 */
expopnode *alloc_binop(token_type op, expopnode *left, expopnode *right)
{
    binop *bop = safe_calloc(1, sizeof(binop));
    bop->opnode.free = &free_binop;
    
    switch (op) {
    case TOK_LESSTHAN:
        bop->opnode.evaluate = &eval_less;
        break;
        
    default:
        assert(0);
    }
    
    return &bop->opnode;
}

/* Free unary op code
 */
void free_unop(expopnode *node)
{
    unop *uop = (unop *)node;
    
    if (uop->value) {
        uop->value->free(uop->value);
    }
    
    free(node);
}

/* Allocate unary op
 */
expopnode *alloc_unop(token_type op, expopnode *value)
{
    assert(op == TOK_MINUS);
    
    unop *uop = safe_calloc(1, sizeof(unop));
    
    uop->opnode.free = &free_unop;
    uop->opnode.evaluate = &eval_unary_minus;
    
    return &uop->opnode;
}

/* Evaluate a literal
 */
value *eval_literal(expopnode *node)
{
    litop *lop = (litop *)node;
    return lop->literal;
}

/* Free a literal
 */
void free_litop(expopnode *node)
{
    litop *lop = (litop *)node;
    value_free(lop->literal);
    free(node);
}

/* Allocate a literal
 */
expopnode *alloc_literal(value *value)
{
    litop *lop = calloc(1, sizeof(litop));
    lop->opnode.free = &free_litop;
    lop->opnode.evaluate = &eval_literal;
    lop->literal = value;
    return &lop->opnode;
}

/* evalute a variable reference
 */
value *eval_varref(expopnode *node)
{
    return NULL;
}

/* free a variable reference
 */
void free_varref(expopnode *node)
{
    varref *var = (varref *)node;
    free(var->varname);
    free(var);
}

/* allocate a variable reference
 */
expopnode *alloc_varref(char *varname)
{
    varref *var = calloc(1, sizeof(varref));
    
    var->opnode.free = &free_varref;
    var->opnode.evaluate = &eval_varref;
    var->varname = varname;
    
    return &var->opnode;
}

