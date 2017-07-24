#ifndef run_h
#define run_h

typedef struct parser parser;
typedef struct statement statement;

extern void run_parse(parser *prs, statement *stmt);

#endif /* run_h */
