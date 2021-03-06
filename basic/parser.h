#ifndef parser_h
#define parser_h

#include <setjmp.h>
#include <stdio.h>

typedef struct program program;
typedef struct parser parser;
typedef struct statement statement;
typedef enum source source;
typedef enum token_type token_type;
typedef struct value value;

enum token_type
{
    TOK_END,
    TOK_ERROR,
    
    TOK_NUMBER,
    TOK_IDENTIFIER,
    TOK_STRING,
    
    /* do not change ordering - must be operators from here on
     * with relops grouped at the end
     */
    TOK_PLUS,
    TOK_FIRSTOP = TOK_PLUS,
    TOK_MINUS,
    TOK_TIMES,
    TOK_DIVIDE,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_COMMA,
    TOK_SEMICOLON,
    
    TOK_EQUALS,
    TOK_FIRSTRELOP = TOK_EQUALS,
    TOK_LESSTHAN,
    TOK_GREATERTHAN,
    TOK_LESSEQUALS,
    TOK_GREATEREQUALS,
    TOK_NOTEQUALS,
    TOK_LASTOP = TOK_NOTEQUALS,
};

enum source
{
    SRC_FILE,
    SRC_REPL
};

static inline int is_operator(token_type type)
{
    return (type >= TOK_FIRSTOP && type <= TOK_LASTOP);
}

static inline int is_relop(token_type type)
{
    return (type >= TOK_FIRSTRELOP && type <= TOK_LASTOP);
}

struct parser
{
    char *line_buffer;
    int in_line_buffer;
    int line_buffer_size;
    int parse_index;
    int token_start;
    int token_end;
    enum token_type token_type;
    
    char *error_msg;
};

static inline int parser_error(parser *prs)
{
    return prs->error_msg != NULL;
}

extern parser *parser_alloc();
extern void parser_free(parser *p);
extern int parser_parse_file(parser *prs, FILE *fp, program *pgm);
extern int parser_parse_repl_line(parser *prs, char *line, program *pgm, statement **stmt);
extern void parse_next_token(parser *prs);
extern char *parser_extract_token_text(parser *prs);
extern void parser_set_error(parser *prs, const char *fmt, ...);
extern char *parser_describe_token(parser *prs);
extern int parser_expect_id(parser *prs, const char *id);
extern int parser_expect_operator(parser *prs, token_type token);
extern value *parser_expect_number(parser *prs);
extern char *parser_expect_var(parser *prs);
extern int parser_expect_line_no(parser *prs, int previous_token);
extern char *parser_expect_filename(parser *prs);
extern int parser_expect_end_of_line(parser *prs);



#endif /* parser_h */
