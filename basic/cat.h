#ifndef cat_h
#define cat_h

typedef struct parser parser;
typedef struct statement statement;

extern void cat_parse(parser *prs, statement *stmt);

#endif /* cat_h */

