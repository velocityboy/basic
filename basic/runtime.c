#include "program.h"
#include "runtime.h"
#include "safemem.h"
#include "statement.h"

struct runtime
{
    int dummy;
};

/* Allocate a runtime environment
 */
runtime *runtime_alloc()
{
    runtime *rt = calloc(1, sizeof(runtime));
    return rt;
}

/* Free a runtime environment
 */
void runtime_free(runtime *rt)
{
    free(rt);
}

/* Run a program
 */
void runtime_run(runtime *rt, program *pgm)
{
    statement *stmt = pgm->head;
    
    while (stmt)
    {
        stmt->body->execute(stmt->body);
        stmt = stmt->next;
    }
}
