#ifndef let_h
#define let_h

typedef struct parser parser;
typedef struct statement statement;

extern void let_parse(parser *prs, statement *stmt);

#endif /* let_h */
