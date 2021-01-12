/* symbol table */
struct symbol {
    char *name;
    char type;
};

/* simple symtab of fixed size */
#define NHASH 9997
struct symbol symtab[NHASH];

struct symbol *lookup(char*, int);

/* list of symbols, for an argument list */
struct symlist {
    struct symbol *sym;
    struct symlist *next;
};

struct symlist *newsymlist(char *sym, struct symlist *next);
void symlistsettype(char type, struct symlist *sl);
void symlistfree(struct symlist *sl);

/* node types
 *  P program root node
 *  L list of declarations or statements
 *  B boolean declaration
 *  D decimal declaration
 *  F loop for
 *  I iterator
 *  E iterator list
 *  = assignment
 *  + - * / & | ^ !
 *  1-3 comparison ops, bit coded 03 equal, 02 less, 01 greater
 *  K number const
 *  N symbol ref
 */

/* nodes in the Abstract Syntax Tree */

struct ast
{
    int nodetype;

    struct ast *l;
    struct ast *r;
};

struct numval {
    int nodetype;			/* type K */
    int number;
};

struct symref {
    int nodetype;			/* type N */
    struct symbol *s;
};

struct symasgn {
    int nodetype;			/* type = */
    struct ast *s;          /* variable */
    struct ast *v;		    /* value */
};

struct declaration
{
    int nodetype;           /* type B or D */
    struct symlist *symlist;
};

/* build an AST */
struct ast *newast(int nodetype, struct ast *l, struct ast *r);
struct ast *newcmp(int cmptype, struct ast *l, struct ast *r);
struct ast *newref(char *s);
struct ast *newasgn(struct ast *s, struct ast *v);
struct ast *newnum(int d);
struct ast *newdeclar(char type, struct symlist *symlist);

/* delete and free an AST */
void treefree(struct ast *);

/* interface to the lexer */
extern int yylineno;
int yyparse();
int yylex();
void yyerror (char *s, ...);

int dumpast(struct ast * a, int level, const char * description);