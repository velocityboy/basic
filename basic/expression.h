#ifndef expression_h
#define expression_h

typedef struct expopnode expopnode;
typedef struct expression expression;
typedef struct parser parser;
typedef struct value value;

struct expopnode
{
    value *(*evaluate)(expopnode *node);
    void (*free)(expopnode *node);
};

expression *expression_parse(parser *prs);
void expression_free(expression *exp);
value *expression_evaluate(expression *exp);

#endif /* expression_h */
