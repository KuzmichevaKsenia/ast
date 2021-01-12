#  include <stdio.h>
#  include <stdlib.h>
#  include <stdarg.h>
#  include <string.h>
#  include <math.h>
#  include "interface.h"

/* symbol table */
/* hash a symbol */
static unsigned symhash(char *sym) {
    unsigned int hash = 0;
    unsigned c;

    while (c = *sym++) hash = hash * 9 ^ c;

    return hash;
}

/*
 * Ищет по хеш-таблице переменную, имя которой передается в первом параметре.
 * Возвращает структуру, в которой хранится имя и тип переменной.
 * По второму параметру определяется, было ли обращение к хеш-таблице декларированием переменной или вызов существующей.
 * */
struct symbol *lookup(char *sym, int isnew) {
    struct symbol *sp = &symtab[symhash(sym) % NHASH];
    int scount = NHASH;        /* how many have we looked at */

    while (--scount >= 0) {
        if (sp->name && !strcmp(sp->name, sym)) {
            if (isnew) yyerror("var name '%s' declared twice", sym);
            else return sp;
        }

        if (!sp->name) {        /* new entry */
            if(isnew) {
                sp->name = strdup(sym);
                sp->type = ' ';
                return sp;
            } else yyerror("non declared var '%s'", sym);
        }

        if (++sp >= symtab + NHASH) sp = symtab; /* try the next entry */
    }
    yyerror("symbol table overflow\n");
}

/*
 * Создает новый узел AST.
 * */
struct ast *newast(int nodetype, struct ast *l, struct ast *r) {
    struct ast *a = malloc(sizeof(struct ast));

    if (a == NULL) yyerror("out of space");

    a->nodetype = nodetype;
    a->l = l;
    a->r = r;
    return a;
}

/*
 * Создает новый узел AST для целочисленной константы.
 * */
struct ast *newnum(int d) {
    struct numval *a = malloc(sizeof(struct numval));

    if (a == NULL) yyerror("out of space");
    a->nodetype = 'K';
    a->number = d;
    return (struct ast *) a;
}

/*
 * Создает новый узел AST для оператора сравнения
 * */
struct ast *newcmp(int cmptype, struct ast *l, struct ast *r) {
    struct ast *a = malloc(sizeof(struct ast));

    if (a == NULL) yyerror("out of space");

    a->nodetype = '0' + cmptype;
    a->l = l;
    a->r = r;
    return a;
}

/*
 * Создает новый узел AST для вызова переменной, где s – имя переменной.
 * */
struct ast *newref(char *s) {
    struct symref *a = malloc(sizeof(struct symref));

    if (a == NULL) yyerror("out of space");

    a->nodetype = 'N';
    a->s = lookup(s, 0);
    return (struct ast *) a;
}

/*
 * Создает новый узел AST для оператора присваивания, где s – узел переменной, v – узел присваиваемого выражения.
 * */
struct ast *newasgn(struct ast *s, struct ast *v) {
    struct symasgn *a = malloc(sizeof(struct symasgn));

    if (a == NULL) yyerror("out of space");

    a->nodetype = '=';
    a->s = s;
    a->v = v;
    return (struct ast *) a;
}

/*
 * Создает узел списка декларируемых переменных.
 * */
struct symlist *newsymlist(char *ch, struct symlist *next) {
    struct symlist *sl = malloc(sizeof(struct symlist));

    if (sl == NULL) yyerror("out of space");

    sl->sym = lookup(ch, 1);
    sl->next = next;
    return sl;
}

/*
 * Устанавливает тип переменным из списка при декларировании.
 * */
void symlistsettype(char type, struct symlist *sl) {
    if (sl == NULL) return;
    sl->sym->type = type;
    symlistsettype(type, sl->next);
}

/*
 * Удаляет список декларируемых переменных.
 * */
void symlistfree(struct symlist *sl) {
    struct symlist *nsl;

    while (sl) {
        nsl = sl->next;
        free(sl);
        sl = nsl;
    }
}

/*
 * Создает новый узел декларирования, где type – тип переменных (‘B’ – Boolean, ‘D’ – Decimal), symlist – список переменных.
 * */
struct ast *newdeclar(char type, struct symlist *symlist) {
    struct declaration *d = malloc(sizeof(struct declaration));

    if (d == NULL) yyerror("out of space");

    symlistsettype(type, symlist);

    d->nodetype = type;
    d->symlist = symlist;
    return (struct ast *) d;
}

/*
 * Удаляет AST.
 * */
void treefree(struct ast *a) {

    switch (a->nodetype) {

        /* two subtrees */
        case 'P':
        case 'F':
        case 'I':
        case 'E':
        case '+':
        case '-':
        case '*':
        case '/':
        case '&':
        case '|':
        case '^':
        case '1':
        case '2':
        case '3':
            treefree(a->r);

            /* one subtree */
        case '!':
            treefree(a->l);

            /* no subtree */
        case 'K':
        case 'N':
            break;

        case 'L':
            free(a->l);
            if (a->r != NULL) {
                treefree(a->r);
            }
            break;

        case 'B':
        case 'D':
            free(((struct declaration *) a)->symlist);
            break;

        case '=':
            free(((struct symasgn *) a)->v);
            break;

        default:
            printf("internal error: free bad node %c\n", a->nodetype);
    }

    free(a);
}

/*
 * Функция вывода ошибки.
 * */
void yyerror(char *s, ...) {
    va_list ap;
    va_start(ap, s);

    fprintf(stderr, "error in line %d: ", yylineno);
    vfprintf(stderr, s, ap);
    fprintf(stderr, "\n");
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc < 2) yyerror("\nNot enough arguments. Please specify filename. \n");
    extern FILE *yyin;
    if ((yyin = fopen(argv[1], "r")) == NULL) yyerror("\nCannot open file %s.\n", argv[1]);
    return yyparse();
}

void drawOffset(int offset) {
    printf("\033[%dC", offset);
}

void drawLine(int offset, int height) {
    printf("\033[%dA", height); // move cursor up by height lines
    printf("\033[%dC", offset);

    for (int i = 0; i < height; i++) {
        printf("|");
        printf("\033[1B\033[1D");
    }
    printf("\r");
}

int drawNode(int parentOffset, const char *nodeName, const char *nodeDescription, const char *branchDescription) {
    char branchStrBuffer[255];

    drawOffset(parentOffset);
    printf("|\n");

    drawOffset(parentOffset);
    if (branchDescription) {
        sprintf(branchStrBuffer, "|-[%s]->(%s){%s}\n", branchDescription, nodeName, nodeDescription);
    } else {
        sprintf(branchStrBuffer, "|-->(%s){%s}\n", nodeName, nodeDescription);
    }

    printf("%s", branchStrBuffer);

    return parentOffset + strlen(branchStrBuffer) - strlen(nodeName) / 2 - strlen(nodeDescription) - 4;
}

/*
 * Функция вывода AST.
 * */
int dumpast(struct ast *a, int offset, const char *description) {
    if (offset == 0) {
        printf("prog\n");
        offset = 2;
    }

    int branchesHeight = 0;

    switch (a->nodetype) {
        case 'P': {
            int childrenOffset = drawNode(offset, "declars", "", NULL);
            branchesHeight += dumpast(a->l, childrenOffset, "");
            drawLine(offset, branchesHeight);
            childrenOffset = drawNode(offset, "stmts", "", NULL);
            branchesHeight += dumpast(a->r, childrenOffset, "");
            branchesHeight += 2;
            break;
        }
        case 'L': {
            branchesHeight += dumpast(a->l, offset, NULL);
            if (a->r == NULL) break;
            drawLine(offset, branchesHeight);
            branchesHeight += dumpast(a->r, offset, NULL);
            break;
        }
        case 'B': {
            int childrenOffset = drawNode(offset, "var names", "", "Boolean");
            struct symlist *sl = ((struct declaration *) a)->symlist;
            do {
                drawNode(childrenOffset, sl->sym->name, "", NULL);
                branchesHeight += 2;
            } while (sl = sl->next);

            drawLine(childrenOffset, branchesHeight);
            branchesHeight += 2;
            break;
        }
        case 'D': {
            int childrenOffset = drawNode(offset, "var names", "", "Decimal");
            struct symlist *sl = ((struct declaration *) a)->symlist;
            do {
                drawNode(childrenOffset, sl->sym->name, "", NULL);
                branchesHeight += 2;
            } while (sl = sl->next);

            drawLine(childrenOffset, branchesHeight);
            branchesHeight += 2;
            break;
        }
        case '=': {
            int childrenOffset = drawNode(offset, ":=", "assignment", description);
            branchesHeight += dumpast(((struct symasgn *) a)->s, childrenOffset, "var");
            drawLine(childrenOffset, branchesHeight);
            branchesHeight += dumpast(((struct symasgn *) a)->v, childrenOffset, "exp");
            branchesHeight += 2;
            break;
        }
        case '+':
        case '-':
        case '*':
        case '/':
        case '&':
        case '|':
        case '^': {
            char buf[10];
            sprintf(buf, "%c", a->nodetype);
            int childrenOffset = drawNode(offset, buf, "bin oper", description);
            branchesHeight += dumpast(a->l, childrenOffset, "left exp");
            drawLine(childrenOffset, branchesHeight);
            branchesHeight += dumpast(a->r, childrenOffset, "right exp");
            branchesHeight += 2;
            break;
        }
        case '!': {
            int childrenOffset = drawNode(offset, "!", "un oper", description);
            branchesHeight += dumpast(a->l, childrenOffset, NULL);
            branchesHeight += 2;
            break;
        }
            /* comparisons */
        case '1': {
            int childrenOffset = drawNode(offset, ">", "cmp oper", description);
            branchesHeight += dumpast(a->l, childrenOffset, "left exp");
            drawLine(childrenOffset, branchesHeight);
            branchesHeight += dumpast(a->r, childrenOffset, "right exp");
            branchesHeight += 2;
            break;
        }
        case '2': {
            int childrenOffset = drawNode(offset, "<", "cmp oper", description);
            branchesHeight += dumpast(a->l, childrenOffset, "left exp");
            drawLine(childrenOffset, branchesHeight);
            branchesHeight += dumpast(a->r, childrenOffset, "right exp");
            branchesHeight += 2;
            break;
        }
        case '3': {
            int childrenOffset = drawNode(offset, "==", "cmp oper", description);
            branchesHeight += dumpast(a->l, childrenOffset, "left exp");
            drawLine(childrenOffset, branchesHeight);
            branchesHeight += dumpast(a->r, childrenOffset, "right exp");
            branchesHeight += 2;
            break;
        }
        case 'K': {
            char buf[10];
            sprintf(buf, "%d", ((struct numval *) a)->number);
            drawNode(offset, buf, "const", description);
            branchesHeight += 2;
            break;
        }
        case 'N': {
            char buf[10];
            int childrenOffset = drawNode(offset, "var", "", description);
            branchesHeight += 2;
            sprintf(buf, "%c", ((struct symref *) a)->s->type);
            drawNode(childrenOffset, buf, "type", NULL);
            branchesHeight += 2;
            drawNode(childrenOffset, ((struct symref *) a)->s->name, "name", NULL);
            drawLine(childrenOffset, branchesHeight);
            branchesHeight += 2;
            break;
        }
        case 'F': {
            int childrenOffset = drawNode(offset, "loop", "for", description);

            branchesHeight += dumpast(a->l, childrenOffset, "iter");
            drawLine(childrenOffset, branchesHeight);
            branchesHeight += dumpast(a->r, childrenOffset, "stmt");

            branchesHeight += 2;
            break;
        }
        case 'I': {
            int childrenOffset = drawNode(offset, "", "", description);

            branchesHeight += dumpast(a->l, childrenOffset, "iter var");
            drawLine(childrenOffset, branchesHeight);
            branchesHeight += dumpast(a->r, childrenOffset, "iter list");

            branchesHeight += 2;
            break;
        }
        case 'E': {
            int childrenOffset = drawNode(offset, "", "", description);

            branchesHeight += dumpast(a->l, childrenOffset, "begin val");
            drawLine(childrenOffset, branchesHeight);
            branchesHeight += dumpast(a->r, childrenOffset, "end val");

            branchesHeight += 2;
            break;
        }

        default:
            printf("internal error: bad node %c\n", a->nodetype);
    }

    return branchesHeight;
}