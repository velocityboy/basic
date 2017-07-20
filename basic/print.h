#ifndef print_h
#define print_h

typedef struct parser parser;
typedef struct statement statement;

extern void print_parse(parser *prs, statement *stmt);

#endif /* print_h */
