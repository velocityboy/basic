#ifndef input_h
#define input_h

typedef struct parser parser;
typedef struct statement statement;

extern void input_parse(parser *prs, statement *stmt);

#endif /* input_h */
