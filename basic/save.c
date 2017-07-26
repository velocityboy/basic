#include <stdio.h>

#include "expression.h"
#include "parser.h"
#include "program.h"
#include "runtime.h"
#include "safemem.h"
#include "save.h"
#include "statement.h"

typedef struct save_node save_node;

struct save_node
{
    statement_body body;
    char *filename;
};

static void save_execute(statement_body *body, runtime *rt);
static void save_free(statement_body *body);

/* Parse the save statement
 */
void save_parse(parser *prs, statement *stmt)
{
    save_node *save = safe_calloc(1, sizeof(save_node));
    
    if ((save->filename = parser_expect_filename(prs)) == NULL ||
        !parser_expect_end_of_line(prs)) {
        save_free(&save->body);
        return;
    }
    
    save->body.execute = &save_execute;
    save->body.free = &save_free;
    stmt->body = &save->body;
}

/* execute a save node
 */
void save_execute(statement_body *body, runtime *rt)
{
    save_node *save = (save_node *)body;
    
    FILE *fp = fopen(save->filename, "w");
    if (fp == NULL) {
        runtime_set_error(rt, "COULD NOT OPEN %s FOR SAVE", save->filename);
        return;
    }
    
    program *pgm = runtime_get_program(rt);
    for (statement *stmt = pgm->head; stmt; stmt = stmt->next) {
        fprintf(fp, "%s\n", stmt->text);        
    }
    
    fclose(fp);
}

/* free a save node
 */
void save_free(statement_body *body)
{
    save_node *save = (save_node *)body;
    if (save) {
        free(save->filename);
    }
    
    free(body);
}


