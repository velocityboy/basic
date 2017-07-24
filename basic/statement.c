#include "safemem.h"
#include "statement.h"

/* Allocate an empty statement
 */
statement *statement_alloc()
{
    statement *stmt = (statement *)safe_calloc(1, sizeof(statement));
    // stmt->next = NULL;
    // stmt->prev = NULL;
    stmt->line = -1;
    
    return stmt;
}

/* Free a statment, include the body
 */
void statement_free(statement *stmt)
{
    if (stmt == NULL) {
        return;
    }
    
    free(stmt->text);

    if (stmt->body) {
        stmt->body->free(stmt->body);
    }
    
    free(stmt);
}
