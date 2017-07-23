#ifndef gosub_h
#define gosub_h

typedef struct parser parser;
typedef struct statement statement;

extern void gosub_parse(parser *prs, statement *stmt);
extern void return_parse(parser *prs, statement *stmt);

#endif /* gosub_h */
