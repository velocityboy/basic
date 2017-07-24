#ifndef statement_h
#define statement_h

typedef struct runtime runtime;
typedef struct statement statement;
typedef struct statement_body statement_body;

struct statement_body
{
    void (*execute)(statement_body *body, runtime *rt);
    void (*free)(statement_body *body);
};

struct statement
{
    statement *prev;
    statement *next;
    char *text;
    int line;
    statement_body *body;
};

extern statement *statement_alloc();
extern void statement_free(statement *stmt);

#endif /* statement_h */
