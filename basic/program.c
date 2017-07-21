#include <stdio.h>
#include <stdlib.h>

#include "program.h"
#include "safemem.h"
#include "statement.h"

static statement *program_find_statment(program *pgm, int line);

/* Allocate an empty program
 */
program *program_alloc()
{
    program *pgm = safe_calloc(1, sizeof(program));
    pgm->head = NULL;
    pgm->tail = NULL;
    
    return pgm;
}

/* Free all program code
 */
void program_free(program *pgm)
{
    statement *curr = pgm->head;
    statement *next = NULL;
    
    while (curr) {
        next = curr;
        statement_free(curr);
        curr = next;
    }
    
    free(pgm);
}

/* Insert a statement
 */
void program_insert_statement(program *pgm, statement *stmt)
{
    if (stmt->line <= 0) {
        return;
    }
    
    statement *existing = program_find_statment(pgm, stmt->line);
    if (existing && existing->line == stmt->line) {
        /* we need to replace an existing statement */
        statement *prev = existing->prev;
        statement *next = existing->next;
        
        if (prev) {
            prev->next = stmt;
        } else {
            pgm->head = stmt;
        }
        
        if (next) {
            next->prev = stmt;
        } else {
            pgm->tail = stmt;
        }
        
        stmt->prev = prev;
        stmt->next = next;
        
        statement_free(existing);
        return;
    }
    
    /* we are just inserting */
    if (existing) {
        stmt->prev = existing;
        stmt->next = existing->next;
        existing->next = stmt;
        
        if (stmt->next) {
            stmt->next->prev = stmt;
        } else {
            pgm->tail = stmt;
        }
    } else if (pgm->head) {
        stmt->prev = NULL;
        stmt->next = pgm->head;
        
        pgm->head = stmt;
        stmt->next->prev = stmt;
    } else {
        pgm->head = stmt;
        pgm->tail = stmt;
    }
}

/* Search for a statement by line number. Returns the statment at
 * or before the given line number, or NULL if no statment matches
 * that criteria.
 */
statement *program_find_statment(program *pgm, int line)
{
    statement *prev = NULL;
    statement *stmt = NULL;
    
    for (stmt = pgm->head; stmt != NULL; prev = stmt, stmt = stmt->next) {
        if (stmt->line >= line) {
            break;
        }
    }
    
    return prev;
}
