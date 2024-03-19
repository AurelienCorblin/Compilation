%{
#include "tree.h"
#include "le_parser.h"
#include <stdio.h>
#include <string.h>
int lineno = 1;
int nbChar = 1;
%}

%option nounput
%option noinput
%option noyywrap

%x COMMENTARY

%%
\/\/.* 					;
"/*"  					BEGIN COMMENTARY;

<COMMENTARY>"*/" 		BEGIN INITIAL;
<COMMENTARY>.   		nbChar++;
<COMMENTARY>\n  		{ lineno++; nbChar = 1;}

int                  	{strcpy(yylval.type, yytext); nbChar += yyleng; return TYPE;}
char                 	{strcpy(yylval.type, yytext); nbChar += yyleng; return TYPE;}
void                 	{nbChar += yyleng; return VOID;}
if                   	{nbChar += yyleng; return IF;}
else                 	{nbChar += yyleng; return ELSE;}
return               	{nbChar += yyleng; return RETURN;}
while                	{nbChar += yyleng; return WHILE;}

[a-zA-Z_][a-zA-Z0-9_]* 	{strcpy(yylval.ident, yytext); nbChar += yyleng; return IDENT;}
[0-9]+              	{yylval.num = atoi(yytext); nbChar += yyleng; return NUM; }

"=="                	{strcpy(yylval.comp, yytext); nbChar += yyleng; return EQ;} 
"!="                	{strcpy(yylval.comp, yytext); nbChar += yyleng; return EQ;}
"<"|">="|"<="|">"       {strcpy(yylval.comp, yytext); nbChar += yyleng; return ORDER;}
"&&"                	{nbChar += yyleng; return AND;}
"||"                	{nbChar += yyleng; return OR;}
"+"|"-"                 {yylval.byte = yytext[0]; nbChar += yyleng; return ADDSUB;}
"*"|"%"|"/"             {yylval.byte = yytext[0]; nbChar += yyleng; return DIVSTAR;}

'\\['nt]'          		{yylval.byte = yytext[0]; return CHARACTER;}
'.'                		{yylval.byte = yytext[0]; return CHARACTER;}

[ \t]+              	;
\n                  	{lineno++; nbChar = 1;}
.                   	{nbChar++; return yytext[0];}
%%