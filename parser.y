%{
#  include <stdio.h>
#  include <stdlib.h>
#  include "interface.h"
%}

%union {
  struct ast *a;
  int d;
  struct symlist *sl;
  int fn;
  char *ch;
}

/* declare tokens */
%token <d> NUMBER
%token <ch> NAME
%token BN ED VAR BOOL DEC ASSIGN FOR TO DO

%nonassoc <fn> CMP
%left '+' '-'
%left '*' '/'
%left '&' '|' '^'
%nonassoc '!'

%type <a> declars list declar stmt iterator exp
%type <sl> symlist

%%
prog: declars BN list ED { struct ast *prog = newast('P', newast('L', $1, NULL), newast('L', $3, NULL));
			dumpast(prog, 0, NULL);
			treefree(prog);
		}
;

declars: declar { $$ = $1; }
	| declar declars { $$ = newast('L', $1, $2); }
;

declar: VAR symlist ':' BOOL { yyerror("';' expected"); }
	| VAR symlist ':' DEC { yyerror("';' expected"); }
	| VAR symlist BOOL { yyerror("':' expected"); }
	| VAR symlist DEC { yyerror("':' expected"); }
	| VAR ':' BOOL { yyerror("variable names expected"); }
	| VAR ':' DEC { yyerror("variable names expected"); }
	| VAR symlist ':' BOOL ';' { $$ = newdeclar('B', $2); }
	| VAR symlist ':' DEC ';' { $$ = newdeclar('D', $2); }
;

symlist: NAME { $$ = newsymlist($1, NULL); }
	| NAME ',' symlist { $$ = newsymlist($1, $3); }
	| NAME symlist { yyerror("',' expected"); }
;

list: stmt { $$ = $1; }
	| stmt list { $$ = newast('L', $1, $2); }
;

stmt: FOR iterator DO stmt { $$ = newast('F', $2, $4); }
	| NAME ASSIGN exp ';' { $$ = newasgn(newref($1), $3); }
	| NAME ASSIGN exp { yyerror("';' expected"); }
	| NAME ASSIGN ';' { yyerror("expression missed"); }
	| ASSIGN { yyerror("variable missed"); }
	| NAME exp { yyerror("':=' expected"); }
;

iterator:  { yyerror("iterator missed"); }
	| NAME ASSIGN exp TO exp { $$ = newast('I', newref($1), newast('E', $3, $5)); }
	| ASSIGN { yyerror("variable missed"); }
	| NAME exp { yyerror("':=' expected"); }
	| NAME ASSIGN TO { yyerror("iterator begin value missed"); }
	| NAME ASSIGN exp TO { yyerror("iterator end value missed"); }
;

exp: exp CMP exp	{ $$ = newcmp($2, $1, $3); }
	| exp '+' exp	{ $$ = newast('+', $1, $3); }
	| exp '-' exp	{ $$ = newast('-', $1, $3); }
	| exp '*' exp	{ $$ = newast('*', $1, $3); }
	| exp '/' exp	{ $$ = newast('/', $1, $3); }
	| exp '&' exp	{ $$ = newast('&', $1, $3); }
	| exp '|' exp	{ $$ = newast('|', $1, $3); }
	| exp '^' exp	{ $$ = newast('^', $1, $3); }
	| '!' exp	{ $$ = newast('!', $2, NULL); }
	| '(' exp ')'	{ $$ = $2; }
	| NUMBER	{ $$ = newnum($1); }
	| NAME		{ $$ = newref($1); }
	| '(' ')'	{ yyerror("expression in brackets missed"); }
;
%%
