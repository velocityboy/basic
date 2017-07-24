#include <stdio.h>

#include "parser.h"
#include "program.h"
#include "runtime.h"
#include "statement.h"

static int run_program(const char *name);
static int run_repl();

int main(int argc, const char * argv[])
{
    if (argc > 1) {
        return run_program(argv[1]);
    }
    
    return run_repl();
}

int run_program(const char *name)
{
    FILE *fp = fopen(name, "r");
    if (!fp) {
        fprintf(stderr, "could not open %s\n", name);
        return 1;
    }

    program *pgm = program_alloc();
    parser *prs = parser_alloc();

    if (parser_parse_file(prs, fp, pgm) == -1) {
        fprintf(stderr, "parse failed.\n");
        return 1;
    }

    runtime *rt = runtime_alloc(pgm);
    runtime_run(rt);
    runtime_free(rt);
    
    return 0;
}

int run_repl()
{
    char input[200];
    
    program *pgm = program_alloc();
    parser *prs = parser_alloc();
    runtime *rt = runtime_alloc(pgm);
    
    while (1) {
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        
        statement *stmt = NULL;
        if (parser_parse_repl_line(prs, input, pgm, &stmt) == 0) {
            continue;
        }
        
        if (stmt != NULL) {
            runtime_execute_statement(rt, stmt);
        }
    }
    
    return 0;
}

