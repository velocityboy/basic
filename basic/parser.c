#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>

#include "keyword.h"
#include "parser.h"
#include "program.h"
#include "safemem.h"
#include "statement.h"
#include "stringutil.h"

static inline char parser_peek(parser *prs)
{
    return prs->line_buffer[prs->parse_index];
}

static inline void parser_next(parser *prs)
{
    if (prs->line_buffer[prs->parse_index]) {
        prs->parse_index++;
    }
}

static inline char parser_get(parser *prs)
{
    char ch = prs->line_buffer[prs->parse_index];
    if (ch) {
        prs->parse_index++;
    }
    return ch;
}

static statement *parse_statement(parser *prs, int from_repl);
static void parse_line_number(parser *prs, statement *stmt);
static void parse_identifier(parser *prs);
static void parse_number(parser *prs);
static void parse_string(parser *prs);
static void parse_operator(parser *prs);
static void throw_error(parser *prs, const char *fmt, ...) __attribute__((__noreturn__));
static int read_line(parser *prs, FILE *fp);
static void append_line_buffer(parser *prs, char ch);
static void grow_line_buffer(parser *prs);

/* Allocate and initialize a parser
 */
parser *parser_alloc()
{
    parser *prs = (parser*)safe_calloc(1, sizeof(parser));
    prs->line_buffer_size = 80;
    prs->line_buffer = safe_calloc(prs->line_buffer_size, sizeof(char));
    return prs;
}

/* Free a parser
 */
void parser_free(parser *prs)
{
    free(prs->line_buffer);
    free(prs);
}

/* Parse lines from a file into the given program. As this is not
 * from REPL, every statement is expected to have a line number and
 * REPL-only keywords are not allowed.
 *
 * Returns -1 on failure, 0 on success, and will report errors to stderr.
 */
int parser_parse_file(parser *prs, FILE *fp, program *pgm)
{
    while (read_line(prs, fp) != -1) {
        strtrim(prs->line_buffer);
        if (!prs->line_buffer[0]) {
            continue;
        }
        
        statement *stmt = parse_statement(prs, 0);
        
        if (stmt) {
            program_insert_statement(pgm, stmt);
        }
    }

    return 0;
}

/* Parse and return a statement
 */
statement *parse_statement(parser *prs, int from_repl)
{
    statement *stmt = statement_alloc();
    
    if (setjmp(prs->error) != 0) {
        if (prs->error_msg) {
            fprintf(stderr, "%s", prs->error_msg);
            if (stmt->line != -1) {
                fprintf(stderr, " IN LINE %d", stmt->line);
            }
            fprintf(stderr, "\n");
        }
    
        statement_free(stmt);
        return NULL;
    }

    prs->parse_index = 0;

    parse_line_number(prs, stmt);
    
    parse_next_token(prs);
    
    keyword *kw = NULL;
    if (prs->token_type == TOK_IDENTIFIER) {
        char *text = parser_extract_token_text(prs);
        kw = kw_find(text);
        free(text);
    }
    
    // TODO a line number with nothing behind it means to delete the line
    
    if (kw == NULL) {
        throw_error(prs, "KEYWORD EXPECTED");
    }
    
    if (from_repl && (kw->flags & KWFL_OK_IN_REPL) == 0) {
        throw_error(prs, "'%s' IS ONLY VALID IN A PROGRAM", kw->id);
    } else if (!from_repl && (kw->flags & KWFL_OK_IN_STMT) == 0) {
        throw_error(prs, "'%s' IS NOT VALID IN A PROGRAM", kw->id);
    }
    
    parse_next_token(prs);
    kw->parse_statement(prs, stmt);
    
    return stmt;
}

/* Parse a line number, if there is one, and set it into the statement
 */
void parse_line_number(parser *prs, statement *stmt)
{
    if (!isdigit(parser_peek(prs))) {
        return;
    }
    
    int line = 0;
    while(1) {
        char ch = parser_peek(prs);
        if (!isdigit(ch)) {
            break;
        }
        
        line = (line * 10) + (ch - '0');
        
        parser_next(prs);
    }
    
    stmt->line = line;
}

/* Make a copy of the current token text
 */
char *parser_extract_token_text(parser *prs)
{
    size_t len = prs->token_end - prs->token_start;
    char *text = safe_calloc(len + 1, sizeof(char));
    memcpy(text, prs->line_buffer + prs->token_start, len);
    text[len] = '\0';
    return text;
}

/* Parse the next token out of the input line
 */
void parse_next_token(parser *prs)
{
    while (isspace(parser_peek(prs))) {
        parser_next(prs);
    }
    
    prs->token_start = prs->parse_index;
    
    if (parser_peek(prs) == '\0') {
        prs->token_type = TOK_END;
    } else if (isalpha(parser_peek(prs))) {
        parse_identifier(prs);
    } else if (isdigit(parser_peek(prs))) {
        parse_number(prs);
    } else if (parser_peek(prs) == '"') {
        parse_string(prs);
    } else {
        parse_operator(prs);
    }
    
    prs->token_end = prs->parse_index;
}

/* Parse an identifier (which might end with $ if it's a variable*
 */
void parse_identifier(parser *prs)
{
    prs->token_type = TOK_IDENTIFIER;
        
    while (isalpha(parser_peek(prs))) {
        parser_next(prs);
    }
        
    if (parser_peek(prs) == '$') {
        parser_next(prs);
    }
}

/* Parse a number literal (standard floating point 1.0E+09)
 */
void parse_number(parser *prs)
{
    prs->token_type = TOK_NUMBER;
    
    while (isdigit(parser_peek(prs))) {
        parser_next(prs);
    }
    
    if (parser_peek(prs) == '.') {
        parser_next(prs);
        while (isdigit(parser_peek(prs))) {
            parser_next(prs);
        }
    }
    
    if (parser_peek(prs) == 'e' || parser_peek(prs) == 'E') {
        parser_next(prs);
        
        if (parser_peek(prs) == '+' || parser_peek(prs) == '-') {
            parser_next(prs);
        }
        
        if (!isdigit(parser_peek(prs))) {
            throw_error(prs, "INVALID NUMBER");
        }
        
        while (isdigit(parser_peek(prs))) {
            parser_next(prs);
        }
    }
}

/* Parse a string literal
 */
void parse_string(parser *prs)
{
    prs->token_type = TOK_STRING;

    parser_next(prs);
    while (1) {
        char ch = parser_get(prs);
        if (ch == '"') {
            break;
        }
        
        if (!ch) {
            throw_error(prs, "UNTERMINATED STRING");
        }
    }
}

/* Parse operator
 */
void parse_operator(parser *prs)
{
    char ch = parser_get(prs);
    
    switch (ch) {
        case '+':
            prs->token_type = TOK_PLUS;
            break;
    
        case '-':
            prs->token_type = TOK_MINUS;
            break;
    
        case '*':
            prs->token_type = TOK_TIMES;
            break;
            
        case '/':
            prs->token_type = TOK_DIVIDE;
            break;
    
        case '(':
            prs->token_type = TOK_LPAREN;
            break;
    
        case ')':
            prs->token_type = TOK_RPAREN;
            break;
    
        case '=':
            prs->token_type = TOK_EQUALS;
            break;
            
        case ',':
            prs->token_type = TOK_COMMA;
            break;
            
        case ';':
            prs->token_type = TOK_SEMICOLON;
            break;
    
        case '<':
            if (parser_peek(prs) == '=') {
                prs->token_type = TOK_LESSEQUALS;
                parser_next(prs);
            } else if (parser_peek(prs) == '>') {
                prs->token_type = TOK_NOTEQUALS;
                parser_next(prs);
            } else {
                prs->token_type = TOK_LESSTHAN;
            }
            break;
    
        case '>':
            if (parser_peek(prs) == '=') {
                prs->token_type = TOK_GREATEREQUALS;
                parser_next(prs);
            } else {
                prs->token_type = TOK_GREATERTHAN;
            }
            break;
            
        default:
            throw_error(prs, "INVALID OPERATOR %c", ch);
    }
}

/* Report an error and throw an exception
 */
void throw_error(parser *prs, const char *fmt, ...)
{
    char error[100];
    va_list args;
    va_start(args, fmt);
    vsnprintf(error, sizeof(error), fmt, args);
    va_end(args);
    
    prs->error_msg = safe_strdup(error);
    longjmp(prs->error, 1);
}

/* Read a line from the given file. Returns the number of characters
 * read or -1 on EOF.
 */
int read_line(parser *prs, FILE *fp)
{
    int ch;
    
    prs->in_line_buffer = 0;
    
    while ((ch = fgetc(fp)) != EOF) {
        if (ch == '\r' || ch == '\n') {
            if (ch == '\r') {
                ch = fgetc(fp);
                if (ch != EOF && ch != '\n') {
                    ungetc(ch, fp);
                }
            }
            break;
        }
        
        if (ch == EOF) {
            break;
        }
        
        append_line_buffer(prs, ch);
    }
    
    append_line_buffer(prs, '\0');
    
    if (ch == EOF && !prs->line_buffer[0]) {
        return -1;
    }
    
    return prs->in_line_buffer - 1;
}

/* Append a character to the line buffer
 */
void append_line_buffer(parser *prs, char ch)
{
    if (prs->in_line_buffer >= prs->line_buffer_size) {
        grow_line_buffer(prs);
    }
    assert(prs->in_line_buffer < prs->line_buffer_size);
    prs->line_buffer[prs->in_line_buffer++] = ch;
}

/* Increase the size of the read line buffer
 */
void grow_line_buffer(parser *prs)
{
    int new_size = 2 * prs->line_buffer_size;
    prs->line_buffer = safe_realloc(prs->line_buffer, new_size);
    prs->line_buffer_size = new_size;
}

