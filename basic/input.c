#include <string.h>

#include "expression.h"
#include "input.h"
#include "parser.h"
#include "runtime.h"
#include "safemem.h"
#include "statement.h"
#include "stringutil.h"
#include "value.h"

typedef struct input_node input_node;

struct input_node
{
    statement_body body;
    char *prompt;
    char *varname;
};

static void input_execute(statement_body *body, runtime *rt);
static void input_free(statement_body *body);

/* Parse the input statement
 */
void input_parse(parser *prs, statement *stmt)
{
    input_node *inp = safe_calloc(1, sizeof(input_node));
    
    /* the prompt is optional 
     */
    if (prs->token_type == TOK_STRING) {
        inp->prompt = safe_strdup(parser_extract_token_text(prs));
        strunquote(inp->prompt);
        parse_next_token(prs);
        
        if (!parser_expect_operator(prs, TOK_COMMA)) {
            input_free(&inp->body);
            return;
        }
    }
    
    if ((inp->varname = parser_expect_var(prs)) == NULL) {
        input_free(&inp->body);
        return;
    }
    
    inp->body.execute = &input_execute;
    inp->body.free = &input_free;

    stmt->body = &inp->body;
}

/* execute an input node
 */
void input_execute(statement_body *body, runtime *rt)
{
    input_node *inp = (input_node*)body;
    
    printf("%s? ", inp->prompt ? inp->prompt : "");
    
    char input[200];
    int eof = 0;
    
    while (1) {
        if (fgets(input, sizeof(input), stdin) == NULL) {
            if (feof(stdin)) {
                input[0] = '\0';
                eof = 1;
            } else {
                runtime_set_error(rt, "ERROR READING TERMINAL INPUT");
                return;
            }
        }
    
        size_t len = strlen(input);
        for (; len > 0; len--) {
            if (input[len - 1] != '\r' && input[len - 1] != '\n') {
                break;
            }
        }
        input[len] = '\0';
        
        /* TODO factor this check out somewhere
         * or even better store variables as their var index when parsing
         */
        value *value = NULL;
        if (inp->varname[strlen(inp->varname) - 1] == '$') {
            value = value_alloc_string(input, VAL_COPY);
        } else if (eof) {
            /* If we've hit EOF we'll never get a valid number 
             */
            value = value_alloc_number(0);
        } else {
            double num = 0.0;
            
            if (sscanf(input, "%lf", &num) == 0) {
                printf("INVALID INPUT\n");
                continue;
            }
            
            value = value_alloc_number(num);
        }
        
        runtime_setvar(rt, inp->varname, value);
        break;
    }
}

/* free an input node
 */
void input_free(statement_body *body)
{
    input_node *inp = (input_node*)body;
    
    if (inp) {
        free(inp->prompt);
        free(inp->varname);
    }

    free(inp);
}


