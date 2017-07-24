#ifndef list_h
#define list_h

typedef struct parser parser;
typedef struct statement statement;

extern void list_parse(parser *prs, statement *stmt);

#endif /* list_h */
