#include <limits.h>
#include <stdio.h>

#include "expression.h"
#include "load.h"
#include "parser.h"
#include "program.h"
#include "runtime.h"
#include "safemem.h"
#include "statement.h"
#include "value.h"

typedef struct load_node load_node;

struct load_node
{
    statement_body body;
    char *filename;
    parser *parser;
};

static void load_execute(statement_body *body, runtime *rt);
static void load_free(statement_body *body);

/* Parse the load statement
 */
void load_parse(parser *prs, statement *stmt)
{
    load_node *load = safe_calloc(1, sizeof(load_node));
    
    if ((load->filename = parser_expect_filename(prs)) == NULL ||
        !parser_expect_end_of_line(prs)) {
        load_free(&load->body);
        return;
    }
    
    load->parser = prs;
    load->body.free = &load_free;
    load->body.execute = &load_execute;

    stmt->body = &load->body;
}

/* execute a load node
 */
void load_execute(statement_body *body, runtime *rt)
{
    load_node *load = (load_node*)body;
    char path[PATH_MAX];
    
    snprintf(path, sizeof(path), "%s.bas", load->filename);
    
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        runtime_set_error(rt, "FAILED TO OPEN %s", load->filename);
        return;
    }
    
    program *pgm = runtime_get_program(rt);
    program_new(pgm);
    
    if (parser_parse_file(load->parser, fp, pgm) == -1) {
        runtime_set_error(rt, "FAILED TO LOAD %s", load->filename);
    }
}

/* free a load node
 */
void load_free(statement_body *body)
{
    load_node *load = (load_node*)body;
    if (load) {
        free(load->filename);
    }
    
    free(body);
}


