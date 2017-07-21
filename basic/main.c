#include <stdio.h>

#include "parser.h"
#include "program.h"
#include "runtime.h"

int main(int argc, const char * argv[])
{
    if (argc < 2) {
        fprintf(stderr, "%s: filename\n", argv[0]);
        return 1;
    }
    
    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        fprintf(stderr, "could not open %s\n", argv[1]);
        return 1;
    }
    
    program *pgm = program_alloc();
    parser *prs = parser_alloc();
    
    if (parser_parse_file(prs, fp, pgm) == -1) {
        fprintf(stderr, "parse failed.\n");
        return 1;
    }
    
    runtime *rt = runtime_alloc();
    runtime_run(rt, pgm);
    runtime_free(rt);
        
    return 0;
}
