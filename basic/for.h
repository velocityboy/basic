#ifndef for_h
#define for_h

typedef struct parser parser;
typedef struct statement statement;

extern void for_parse(parser *prs, statement *stmt);
extern void next_parse(parser *prs, statement *stmt);

#endif /* for_h */
