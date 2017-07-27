#include <dirent.h>
#include <string.h>
#include <limits.h>

#include "cat.h"
#include "expression.h"
#include "output.h"
#include "parser.h"
#include "runtime.h"
#include "safemem.h"
#include "statement.h"
#include "stringutil.h"
#include "value.h"

typedef struct cat_node cat_node;

struct cat_node
{
    statement_body body;
};

static void cat_execute(statement_body *body, runtime *rt);
static void cat_free(statement_body *body);

/* Parse the cat statement
 */
void cat_parse(parser *prs, statement *stmt)
{
    cat_node *cat = safe_calloc(1, sizeof(cat_node));
    
    cat->body.free = &cat_free;
    cat->body.execute = &cat_execute;

    stmt->body = &cat->body;
}

/* execute a cat node
 */
void cat_execute(statement_body *body, runtime *rt)
{
    int uid = 0;
    char *uidstr = getenv("UID");
    if (uidstr != NULL) {
        sscanf(uidstr, "%d", &uid);
    }
    
    output *out = runtime_get_output(rt);
   
    if (uid != 0) {
        output_print(out, "\nCATALOG FOR USER %9d\n\n", uid);
    }
    
    DIR *dir = opendir(".");
    if (dir == NULL) {
        return;
    }
    
    struct dirent *ent;
    int col = 0;
    const int COLS = 4;
    
    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_type != DT_REG) {
            continue;
        }
        
        char name[PATH_MAX + 1];
        strncpy(name, ent->d_name, PATH_MAX + 1);
        strupr(name);
        
        char *ext = strchr(name, '.');
        if (ext != NULL) {
            *ext++ = '\0';
        } else {
            ext = "";
        }
        
        output_print(out, "%-8s %-3s    ", name, ext);
        
        col = (col + 1) % COLS;
        if (col == 0) {
            putchar('\n');
        }
    }
    
    if (col != 0) {
        putchar('\n');
    }
    
}

/* free a cat node
 */
void cat_free(statement_body *body)
{
    free(body);
}


