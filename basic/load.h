#ifndef load_h
#define load_h

typedef struct parser parser;
typedef struct statement statement;

extern void load_parse(parser *prs, statement *stmt);

#endif /* load_h */
