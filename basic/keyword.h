#ifndef keyword_h
#define keyword_h

#define KWFL_OK_IN_STMT 0x01
#define KWFL_OK_IN_REPL 0x02

typedef struct keyword keyword;
typedef struct parser parser;
typedef struct statement statement;

struct keyword
{
    const char *id;             /* text representatation of keyboard */
    unsigned flags;
    void (*parse_statement)(parser *prs, statement *stmt);
};

extern keyword *kw_find(const char *id);

#endif /* keyword_h */
