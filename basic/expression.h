#ifndef expression_h
#define expression_h

typedef struct expression expression;
typedef struct parser parser;

expression *expression_parse(parser *prs);
void expression_free(expression *exp);

#endif /* expression_h */
