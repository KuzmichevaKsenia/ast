%option noyywrap nodefault yylineno
%{
# include "interface.h"
# include "y.tab.h"
%}

%%
 /* single character ops */
"+" |
"-" |
"*" |
"/" |
"," |
";" |
":" |
"(" |
")"         { return yytext[0]; }

":=" 	    { return ASSIGN; }

 /* comparison ops */
">"         { yylval.fn = 1; return CMP; }
"<"         { yylval.fn = 2; return CMP; }
"=="        { yylval.fn = 3; return CMP; }

 /* logic ops */
".AND."     { return '&'; }
".OR."      { return '|'; }
".XOR."     { return '^'; }
".NOT."     { return '!'; }

 /* keywords */
"Var"       { return VAR; }
"Boolean"   {return BOOL;}
"Decimal"	{return DEC;}
"Begin"     { return BN; }
"End"       { return ED; }
"FOR"       { return FOR; }
"TO"	    { return TO; }
"DO"        { return DO; }

 /* names */
[a-zA-Z]+   { yylval.ch = strdup(yytext); return NAME; }

[0-9]+	    { yylval.d = atoi(yytext); return NUMBER; }

"{"(.|\n)*"}"

[ \t\r\n]
.	{ yyerror("Mystery character '%c'\n", *yytext); }
%%
