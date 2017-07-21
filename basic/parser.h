#ifndef parser_h
#define parser_h

#include <setjmp.h>
#include <stdio.h>

typedef struct program program;
typedef struct parser parser;
typedef enum token_type token_type;

enum token_type
{
    TOK_END,
    TOK_NUMBER,
    TOK_IDENTIFIER,
    TOK_STRING,
    
    TOK_PLUS,
    TOK_MINUS,
    TOK_TIMES,
    TOK_DIVIDE,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_COMMA,
    TOK_SEMICOLON,
    
    TOK_EQUALS,
    TOK_LESSTHAN,
    TOK_GREATERTHAN,
    TOK_LESSEQUALS,
    TOK_GREATEREQUALS,
    TOK_NOTEQUALS
};

static inline int is_relop(token_type type)
{
    return (type >= TOK_EQUALS && type <= TOK_NOTEQUALS);
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
    
    jmp_buf error;
};

extern parser *parser_alloc();
extern void parser_free(parser *p);
extern int parser_parse_file(parser *prs, FILE *fp, program *pgm);
extern void parse_next_token(parser *prs);
extern char *parser_extract_token_text(parser *prs);


#endif /* parser_h */
