#ifndef goto_h
#define goto_h

typedef struct parser parser;
typedef struct statement statement;

extern void goto_parse(parser *prs, statement *stmt);

#endif /* goto_h */

