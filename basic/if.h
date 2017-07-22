#ifndef if_h
#define if_h

typedef struct parser parser;
typedef struct statement statement;

extern void if_parse(parser *prs, statement *stmt);

#endif /* if_h */
