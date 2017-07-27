#include <assert.h>
#include <string.h>

#include "builtins.h"
#include "expression.h"
#include "parser.h"
#include "runtime.h"
#include "safemem.h"
#include "stringutil.h"
#include "value.h"

typedef struct binop binop;
typedef struct binop_argtypes binop_argtypes;
typedef struct funarg funarg;
typedef struct funop funop;
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

struct funarg
{
    funarg *next;
    expression *exp;
};

struct funop
{
    expopnode opnode;
    char *name;
    int args;
    funarg *arglist;
};

struct varref
{
    expopnode opnode;
    char *varname;
};

static int binop_validate(const char *op, valuetype left, valuetype right, binop_argtypes *valid, runtime *rt);
static value *eval_less(expopnode *node, runtime *rt);
static value *eval_greater(expopnode *node, runtime *rt);
static value *eval_lesseq(expopnode *node, runtime *rt);
static value *eval_greatereq(expopnode *node, runtime *rt);
static value *eval_equal(expopnode *node, runtime *rt);
static value *eval_notequal(expopnode *node, runtime *rt);
static value *eval_plus(expopnode *node, runtime *rt);
static value *eval_minus(expopnode *node, runtime *rt);
static value *eval_times(expopnode *node, runtime *rt);
static value *eval_divide(expopnode *node, runtime *rt);
static value *eval_unary_minus(expopnode *node, runtime *rt);

static expopnode *parse_expression(parser *prs);
static expopnode *parse_relop_term(parser *prs);
static expopnode *parse_sum_term(parser *prs);
static expopnode *parse_mul_term(parser *prs);
static expopnode *parse_unary_term(parser *prs);
static expopnode *parse_paren_term(parser *prs);
static expopnode *parse_function_call(parser *prs, char *fn_name);

static void free_binop(expopnode *node);
static expopnode *alloc_binop(token_type op, expopnode *left, expopnode *right);
static void free_unop(expopnode *node);
static expopnode *alloc_unop(token_type op, expopnode *value);

static value *eval_literal(expopnode *node, runtime *rt);
static void free_litop(expopnode *node);
static expopnode *alloc_literal(value *value);

static value *eval_varref(expopnode *node, runtime *rt);
static void free_varref(expopnode *node);
static expopnode *alloc_varref(char *varname);

static void cleanup_funargs(int argc, value **argv);
static value *eval_function(expopnode *node, runtime *rt);
static void free_function(expopnode *node);


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
    if (exp && exp->root) {
        exp->root->free(exp->root);
    }
    free(exp);
}

/* Evaluate an expression
 */
value *expression_evaluate(expression *exp, runtime *rt)
{
    return exp->root->evaluate(exp->root, rt);
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
        value = alloc_unop(TOK_MINUS, value);
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
        ret = alloc_literal(value_alloc_string(text, VAL_ALLOCATED));
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
        parse_next_token(prs);
        if (prs->token_type == TOK_LPAREN) {
            ret = parse_function_call(prs, text);
        } else {
            ret = alloc_varref(text);
        }
        /* do not free(text) - it's owned by either the function call or the varref 
         */
        break;
    
    default:
        text = parser_describe_token(prs);
        parser_set_error(prs, "%s FOUND, LITERAL OR VARIABLE EXPECTED", text);
        free(text);
        break;
    }

    return ret;
}

/* Parse a function call of the form '(' exp [ ',' exp ] * ')'
 */
expopnode *parse_function_call(parser *prs, char *fn_name)
{
    if (!parser_expect_operator(prs, TOK_LPAREN)) {
        return NULL;
    }
    
    funop *fun = safe_calloc(1, sizeof(funop));
    fun->name = fn_name;
    fun->opnode.evaluate = &eval_function;
    fun->opnode.free = &free_function;

    if (prs->token_type == TOK_RPAREN) {
        parse_next_token(prs);
        return &fun->opnode;
    }

    funarg *tail = NULL;
    int valid = 1;
    
    while(1) {
        expression *exp = expression_parse(prs);
        if (exp == NULL) {
            valid = 0;
            break;
        }
        
        fun->args++;
        funarg *arg = safe_calloc(1, sizeof(funarg));
        arg->exp = exp;
        
        if (tail == NULL) {
            fun->arglist = arg;
        } else {
            tail->next = arg;
        }
        tail = arg;
        
        if (prs->token_type == TOK_RPAREN) {
            parse_next_token(prs);
            break;
        } else if (prs->token_type == TOK_COMMA) {
            parse_next_token(prs);
        } else {
            parser_set_error(prs, "SYNTAX ERROR IN FUNCTION CALL");
            valid = 0;
            break;
        }
    }
    
    if (!valid) {
        free_function(&fun->opnode);
        return NULL;
    }
    
    return &fun->opnode;
}


struct binop_argtypes
{
    valuetype left;
    valuetype right;
    int last;
};

static binop_argtypes numbers[] =
{
    { TYPE_NUMBER, TYPE_NUMBER, 1 },
};

static binop_argtypes numbers_and_strings[] =
{
    { TYPE_STRING, TYPE_STRING, 0 },
    { TYPE_NUMBER, TYPE_NUMBER, 1 },
};

/* validate the args that are passed to a binary operator are a compatible
 * type pair
 */
int binop_validate(const char *op, valuetype left, valuetype right, binop_argtypes *valid, runtime *rt)
{
    for (int i = 0; ; i++, valid++)
    {
        if (valid->left == left && valid->right == right) {
            return 1;
        }
    
        if (valid->last) {
            break;
        }
    }
    
    runtime_set_error(rt, "CANNOT %s %s AND %s", op, value_describe_type(left), value_describe_type(right));
    
    return 0;
}

/* runtime for < operator
 */
value *eval_less(expopnode *node, runtime *rt)
{
    binop *bop = (binop *)node;
    value *left = bop->left->evaluate(bop->left, rt);
    value *right = bop->right->evaluate(bop->right, rt);
    
    if (!binop_validate("COMPARE", left->type, right->type, numbers_and_strings, rt)) {
        return NULL;
    }
    
    value *ret = NULL;
    
    if (left->type == TYPE_STRING && right->type == TYPE_STRING) {
        ret = value_alloc_boolean(strcmp(left->string, right->string) < 0);
    } else if (left->type == TYPE_NUMBER && right->type == TYPE_NUMBER) {
        ret = value_alloc_boolean(left->number < right->number);
    }
    
    return ret;
}

/* runtime for > operator
 */
value *eval_greater(expopnode *node, runtime *rt)
{
    binop *bop = (binop *)node;
    value *left = bop->left->evaluate(bop->left, rt);
    value *right = bop->right->evaluate(bop->right, rt);
    
    if (!binop_validate("COMPARE", left->type, right->type, numbers_and_strings, rt)) {
        return NULL;
    }
    
    value *ret = NULL;
    
    if (left->type == TYPE_STRING && right->type == TYPE_STRING) {
        ret = value_alloc_boolean(strcmp(left->string, right->string) > 0);
    } else if (left->type == TYPE_NUMBER && right->type == TYPE_NUMBER) {
        ret = value_alloc_boolean(left->number > right->number);
    }
    
    return ret;
}

/* runtime for <= operator
 */
value *eval_lesseq(expopnode *node, runtime *rt)
{
    binop *bop = (binop *)node;
    value *left = bop->left->evaluate(bop->left, rt);
    value *right = bop->right->evaluate(bop->right, rt);
    
    if (!binop_validate("COMPARE", left->type, right->type, numbers_and_strings, rt)) {
        return NULL;
    }
    
    value *ret = NULL;
    
    if (left->type == TYPE_STRING && right->type == TYPE_STRING) {
        ret = value_alloc_boolean(strcmp(left->string, right->string) <= 0);
    } else if (left->type == TYPE_NUMBER && right->type == TYPE_NUMBER) {
        ret = value_alloc_boolean(left->number <= right->number);
    }
    
    return ret;
}

/* runtime for >= operator
 */
value *eval_greatereq(expopnode *node, runtime *rt)
{
    binop *bop = (binop *)node;
    value *left = bop->left->evaluate(bop->left, rt);
    value *right = bop->right->evaluate(bop->right, rt);
    
    if (!binop_validate("COMPARE", left->type, right->type, numbers_and_strings, rt)) {
        return NULL;
    }
    
    value *ret = NULL;
    
    if (left->type == TYPE_STRING && right->type == TYPE_STRING) {
        ret = value_alloc_boolean(strcmp(left->string, right->string) >= 0);
    } else if (left->type == TYPE_NUMBER && right->type == TYPE_NUMBER) {
        ret = value_alloc_boolean(left->number >= right->number);
    }
    
    return ret;
}

/* runtime for = operator
 */
value *eval_equal(expopnode *node, runtime *rt)
{
    binop *bop = (binop *)node;
    value *left = bop->left->evaluate(bop->left, rt);
    value *right = bop->right->evaluate(bop->right, rt);
    
    if (!binop_validate("COMPARE", left->type, right->type, numbers_and_strings, rt)) {
        return NULL;
    }
    
    value *ret = NULL;
    
    if (left->type == TYPE_STRING && right->type == TYPE_STRING) {
        ret = value_alloc_boolean(strcmp(left->string, right->string) == 0);
    } else if (left->type == TYPE_NUMBER && right->type == TYPE_NUMBER) {
    
        // TODO floating point equality
        ret = value_alloc_boolean(left->number == right->number);
    }
    
    return ret;
}

/* runtime for <> operator
 */
value *eval_notequal(expopnode *node, runtime *rt)
{
    binop *bop = (binop *)node;
    value *left = bop->left->evaluate(bop->left, rt);
    value *right = bop->right->evaluate(bop->right, rt);
    
    if (!binop_validate("COMPARE", left->type, right->type, numbers_and_strings, rt)) {
        return NULL;
    }
    
    value *ret = NULL;
    
    if (left->type == TYPE_STRING && right->type == TYPE_STRING) {
        ret = value_alloc_boolean(strcmp(left->string, right->string) != 0);
    } else if (left->type == TYPE_NUMBER && right->type == TYPE_NUMBER) {
    
        // TODO floating point equality
        ret = value_alloc_boolean(left->number != right->number);
    }
    
    return ret;
}

/* runtime for + operator
 */
value *eval_plus(expopnode *node, runtime *rt)
{
    binop *bop = (binop *)node;
    value *left = bop->left->evaluate(bop->left, rt);
    value *right = bop->right->evaluate(bop->right, rt);
    
    if (!binop_validate("ADD", left->type, right->type, numbers_and_strings, rt)) {
        return NULL;
    }
    
    value *ret = NULL;
    
    if (left->type == TYPE_STRING && right->type == TYPE_STRING) {
        size_t llen = strlen(left->string);
        size_t rlen = strlen(right->string);
        size_t n = llen + rlen + 1;
        char *temp = safe_malloc(n);
        strncpy(temp, left->string, n);
        strncpy(temp + llen, right->string, n - llen);
        ret = value_alloc_string(temp, VAL_COPY);
    } else if (left->type == TYPE_NUMBER && right->type == TYPE_NUMBER) {
        ret = value_alloc_number(left->number + right->number);
    }
    
    return ret;
}

/* runtime for - operator
 */
value *eval_minus(expopnode *node, runtime *rt)
{
    binop *bop = (binop *)node;
    value *left = bop->left->evaluate(bop->left, rt);
    value *right = bop->right->evaluate(bop->right, rt);
    
    if (!binop_validate("SUBTRACT", left->type, right->type, numbers, rt)) {
        return NULL;
    }
    
    return value_alloc_number(left->number - right->number);
}

/* runtime for * operator
 */
value *eval_times(expopnode *node, runtime *rt)
{
    binop *bop = (binop *)node;
    value *left = bop->left->evaluate(bop->left, rt);
    value *right = bop->right->evaluate(bop->right, rt);
    
    if (!binop_validate("TIMES", left->type, right->type, numbers, rt)) {
        return NULL;
    }
    
    return value_alloc_number(left->number * right->number);
}

/* runtime for / operator
 */
value *eval_divide(expopnode *node, runtime *rt)
{
    binop *bop = (binop *)node;
    value *left = bop->left->evaluate(bop->left, rt);
    value *right = bop->right->evaluate(bop->right, rt);
    
    if (!binop_validate("DIVIDE", left->type, right->type, numbers, rt)) {
        return NULL;
    }
    
    return value_alloc_number(left->number / right->number);
}

/* runtime for unary minus
 */
value *eval_unary_minus(expopnode *node, runtime *rt)
{
    unop *uop = (unop *)node;
    
    value *ret = NULL;
    
    value *val = uop->value->evaluate(uop->value, rt);
    
    if (val) {
        if (val->type == TYPE_NUMBER) {
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
    
    if (!bop) {
        return;
    }
    
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
        
    case TOK_GREATERTHAN:
        bop->opnode.evaluate = &eval_greater;
        break;
        
    case TOK_LESSEQUALS:
        bop->opnode.evaluate = &eval_lesseq;
        break;
        
    case TOK_GREATEREQUALS:
        bop->opnode.evaluate = &eval_greatereq;
        break;
        
    case TOK_EQUALS:
        bop->opnode.evaluate = &eval_equal;
        break;
        
    case TOK_NOTEQUALS:
        bop->opnode.evaluate = &eval_notequal;
        break;
        
    case TOK_PLUS:
        bop->opnode.evaluate = &eval_plus;
        break;
        
    case TOK_MINUS:
        bop->opnode.evaluate = &eval_minus;
        break;
        
    case TOK_TIMES:
        bop->opnode.evaluate = &eval_times;
        break;
            
    case TOK_DIVIDE:
        bop->opnode.evaluate = &eval_divide;
        break;
            
    default:
        assert(0);
    }
   
    
    bop->left = left;
    bop->right = right;
    
    return &bop->opnode;
}

/* Free unary op code
 */
void free_unop(expopnode *node)
{
    unop *uop = (unop *)node;
    
    if (uop && uop->value) {
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

    uop->value = value;
    uop->opnode.free = &free_unop;
    uop->opnode.evaluate = &eval_unary_minus;
    
    return &uop->opnode;
}

/* Evaluate a literal
 */
value *eval_literal(expopnode *node, runtime *rt)
{
    litop *lop = (litop *)node;
    return value_clone(lop->literal);
}

/* Free a literal
 */
void free_litop(expopnode *node)
{
    litop *lop = (litop *)node;
    if (lop) {
        value_free(lop->literal);
    }
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
value *eval_varref(expopnode *node, runtime *rt)
{
    varref *var = (varref *)node;
    return value_clone(runtime_getvar(rt, var->varname));
}

/* free a variable reference
 */
void free_varref(expopnode *node)
{
    varref *var = (varref *)node;
    if (var) {
        free(var->varname);
    }
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

/* Clean up arguments evaluated for a function call
 */
void cleanup_funargs(int argc, value **argv)
{
    for (int i = 0; i < argc; ++i) {
        value_free(argv[i]);
    }
    free(argv);
}

/* Evaluate a function call
 */
value *eval_function(expopnode *node, runtime *rt)
{
    funop *fun = (funop *)node;
    value **argv = safe_calloc(fun->args, sizeof(value*));
    
    int argidx = 0;
    for (funarg *arg = fun->arglist; arg; arg = arg->next) {
        if ((argv[argidx++] = expression_evaluate(arg->exp, rt)) == NULL) {
            break;
        }
    }
    
    if (argidx < fun->args) {
        /* argument evaluation failed, which will have already
         * set a runtime error
         */
        cleanup_funargs(fun->args, argv);
        return NULL;
    }
    
    value *ret = builtin_execute(rt, fun->name, fun->args, argv);
    
    cleanup_funargs(fun->args, argv);
    
    return ret;
}

/* Free a function call node
 */
void free_function(expopnode *node)
{
    funop *fun = (funop *)node;
    if (fun == NULL) {
        return;
    }
    
    free(fun->name);
    
    while (fun->arglist) {
        funarg *next = fun->arglist->next;
        expression_free(fun->arglist->exp);
        free(fun->arglist);
        fun->arglist = next;
    }
    
    free(fun);
}
