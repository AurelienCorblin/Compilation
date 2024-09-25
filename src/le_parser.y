%{
#include <stdio.h>
#include "symbol.h"
#include "tree.h"
#include "trad.h"
#include <string.h>
#include <unistd.h>
#include <getopt.h>
int yylex();
extern FILE *yyin;
extern int lineno;
extern int nbChar;
void yyerror(char *msg);
Node* root;
%}

%union {
    char byte;
    int num;
    char ident[64];
    char type[5];
    char comp[3];
    Node *node;
}

%token <byte> CHARACTER ADDSUB DIVSTAR
%token <num> NUM
%token <ident> IDENT 
%token <type> TYPE
%token <comp> ORDER EQ 
%token OR AND IF WHILE ELSE RETURN VOID

%type <node> Prog DeclVars Declarateurs ID 
DeclFoncts DeclFonct EnTeteFonct Parametres 
ListTypVar Corps SuiteInstr Instr Exp TB FB 
M E T F LValue Arguments ListExp

%expect 1

%%
Prog:  DeclVars DeclFoncts
        {
            root = makeNode(Prog, (union Value) {.num = 0}, 0);
            Node* var = makeNode(DeclVars, (union Value) {.num = 0}, 0);
            addChild(root, var);
            addChild(var, $1);
            Node* funct = makeNode(DeclFoncts, (union Value) {.num = 0}, 0);
            addChild(root, funct);
            addChild(funct, $2);
        }
    ;
DeclVars:
      DeclVars TYPE Declarateurs ';'
      { 
            union Value v;
            strcpy(v.str, $2);
            $$ = makeNode(type_, v, 2);
            addChild($$, $3);
            if(NULL != $1) {
                addSibling($1, $$);
                $$ = $1;
            }
      }
    |
      {
            $$ = NULL;
      }
    ;
Declarateurs:
       Declarateurs ',' ID
       {
            $$ = $1;
            addSibling($1, $3);
       }
    |  ID
        {
            $$ = $1;
        }
    ;
ID:
       IDENT
       {
            union Value v;
            strcpy(v.str, $1);
            $$ = makeNode(ident, v, 2);
       }
    |  IDENT '[' NUM ']'
        {
            union Value v;
            strcpy(v.str, $1);
            $$ = makeNode(array, (union Value) {.num = 0}, 0);
            addChild($$, makeNode(ident, v, 2));
            addChild($$, makeNode(size, (union Value) {.num = $3}, 3));
        }
    ;
DeclFoncts:
       DeclFoncts DeclFonct
       {
            $$ = $1;
            addSibling($$, $2);
       }
    |  DeclFonct
        {
            $$ = $1;
        }
    ;
DeclFonct:
       EnTeteFonct Corps
       {
            $$ = makeNode(function, (union Value) {.num = 0}, 0);
            addChild($$, $1);
            addChild($$, $2);
       }
    ;
EnTeteFonct:
       TYPE IDENT '(' Parametres ')'
       {
            union Value v, v1;
            strcpy(v.str, $1);
            strcpy(v1.str, $2);
            $$ = makeNode(heading, (union Value) {.num = 0}, 0);
            addChild($$, makeNode(type_, v, 2));
            addChild($$, makeNode(ident, v1, 2));
            addChild($$, $4);
       }
    |  VOID IDENT '(' Parametres ')'
        {
            union Value v, v1;
            strcpy(v.str, "void");
            strcpy(v1.str, $2);
            $$ = makeNode(heading, (union Value) {.num = 0}, 0);
            addChild($$, makeNode(void_, v, 2));
            addChild($$, makeNode(ident, v1, 2));
            addChild($$, $4);
       }
    ;
Parametres:
       VOID
       {    
            union Value v;
            strcpy(v.str, "void");
            $$ = makeNode(param, (union Value) {.num = 0}, 0);
            addChild($$, makeNode(void_, v, 2));
       }
    |  ListTypVar
        {
            $$ = makeNode(param, (union Value) {.num = 0}, 0);
            addChild($$, $1);
        }
    ;
ListTypVar:
       ListTypVar ',' TYPE IDENT
       {
            $$ = $1;
            union Value v, v1;
            strcpy(v.str, $3);
            strcpy(v1.str, $4);
            Node* tmp = makeNode(type_, v, 2);
            addSibling($$, tmp);
            addChild(tmp, makeNode(ident, v1, 2));
       }
    |  ListTypVar ',' TYPE IDENT'['']'
        {
            $$ = $1;
            union Value v, v1;
            strcpy(v.str, $3);
            strcpy(v1.str, $4);
            Node* tmp = makeNode(type_, v, 2);
            addSibling($$, tmp);
            Node* tmp2 = makeNode(array, (union Value) {.num = 0}, 0);
            addChild(tmp, tmp2);
            addChild(tmp2, makeNode(ident, v1, 2));
       }
    |  TYPE IDENT
        {
            union Value v, v1;
            strcpy(v.str, $1);
            strcpy(v1.str, $2);
            $$ = makeNode(type_, v, 2);
            addChild($$, makeNode(ident, v1, 2));
        }
    |  TYPE IDENT'['']'
        {
            union Value v, v1;
            strcpy(v.str, $1);
            strcpy(v1.str, $2);
            $$ = makeNode(type_, v, 2);
            Node* tmp = makeNode(array, (union Value) {.num = 0}, 0);
            addChild($$, tmp);
            addChild(tmp, makeNode(ident, v1, 2));
        }
    ;
Corps: '{' DeclVars SuiteInstr '}'
        {
            $$ = makeNode(body, (union Value) {.num = 0}, 0);
            Node* var = makeNode(DeclVars, (union Value) {.num = 0}, 0);
            addChild($$, var);
            addChild(var, $2);
            addChild($$, $3);
        }
    ;
SuiteInstr:
       SuiteInstr Instr
       {
            if($1 == NULL) {
                $$ = makeNode(instruction, (union Value) {.num = 0}, 0);
                addChild($$, $2);
            }
            
            if($1 != NULL) {
                if(FIRSTCHILD($1)) {
                    addSibling(FIRSTCHILD($1), $2);
                    $$ = $1;
                }else {
                    addSibling($$, $2);
                    $$ = $1;
                }
            }
       }
    |
        {
            $$ = NULL;
        }
    ;
Instr:
       LValue '=' Exp ';'
       {
            $$ = makeNode(egale, (union Value) {.byte = '='}, 1);
            addChild($$, $1);
            addChild($$, $3);
       }
    |  IF '(' Exp ')' Instr
        {
            union Value v;
            strcpy(v.str, "if");
            $$ = makeNode(if_, v, 2);
            addChild($$, $3);
            if($5 == NULL) {
                Node *tmp = makeNode(instruction, (union Value) {.num = 0}, 0);
                addChild($$, tmp);
                addChild(tmp, NULL);
            }else {addChild($$, $5);}
        }
    |  IF '(' Exp ')' Instr ELSE Instr
        {
            union Value v;
            strcpy(v.str, "if");
            $$ = makeNode(if_, v, 2);
            addChild($$, $3);
            if($5 == NULL) {
                Node *tmp = makeNode(instruction, (union Value) {.num = 0}, 0);
                addChild($$, tmp);
                addChild(tmp, NULL);
            }else {addChild($$, $5);}
            if($7 == NULL) {
                Node *tmp = makeNode(instruction, (union Value) {.num = 0}, 0);
                addChild($$, tmp);
                addChild(tmp, NULL);
            }else {addChild($$, $7);}
        }
    |  WHILE '(' Exp ')' Instr
        {
            union Value v;
            strcpy(v.str, "while");
            $$ = makeNode(while_, v, 2);
            addChild($$, $3);
            if($5 == NULL) {
                Node *tmp = makeNode(instruction, (union Value) {.num = 0}, 0);
                addChild($$, tmp);
                addChild(tmp, NULL);
            }else {addChild($$, $5);}
        }
    |  IDENT '(' Arguments  ')' ';'
        {
            union Value v;
            strcpy(v.str, $1);
            $$ = makeNode(ident_f, v, 2);
            addChild($$, $3);
        }
    |  RETURN Exp ';'
        {
            union Value v;
            strcpy(v.str, "return");
            $$ = makeNode(return_, v, 2);
            addChild($$, $2);
        }
    |  RETURN ';'
        {
            union Value v;
            strcpy(v.str, "return");
            $$ = makeNode(return_, v, 2);
        }
    |  '{' SuiteInstr '}'
        {$$ = $2;}
    |  ';'
        {
            $$ = NULL;
        }
    ;
Exp :  Exp OR TB
        {
            union Value v;
            strcpy(v.str, "||");
            $$ = makeNode(or, v, 2);
            addChild($$, $1);
            addChild($$, $3);
        }
    |  TB
        {
            $$ = $1;
        }
    ;
TB  :  TB AND FB
        {
            union Value v;
            strcpy(v.str, "&&");
            $$ = makeNode(and, v, 2);
            addChild($$, $1);
            addChild($$, $3);
        }
    |  FB
        {
            $$ = $1;
        }
    ;
FB  :  FB EQ M
        {
            union Value v;
            strcpy(v.str, $2);
            $$ = makeNode(eq, v, 2);
            addChild($$, $1);
            addChild($$, $3);
        }
    |  M
        {
            $$ = $1;
        }
    ;
M   :  M ORDER E
        {
            union Value v;
            strcpy(v.str, $2);
            $$ = makeNode(order, v, 2);
            addChild($$, $1);
            addChild($$, $3);
        }
    |  E
        {
            $$ = $1;
        }
    ;
E   :  E ADDSUB T
        {
            $$ = makeNode(addsub, (union Value) {.byte = $2}, 1);
            addChild($$, $1);
            addChild($$, $3);
        }
    |  T
        {
            $$ = $1;
        }
    ;    
T   :  T DIVSTAR F 
        {
            $$ = makeNode(divstar, (union Value) {.byte = $2}, 1);
            addChild($$, $1);
            addChild($$, $3);
        }
    |  F
        {
            $$ = $1;
        }
    ;
F   :  ADDSUB F
        {
            $$ = makeNode(addsub_u, (union Value) {.byte = $1}, 1);
            addChild($$, $2);
        }
    |  '!' F
        {
            $$ = makeNode(not, (union Value) {.byte = '!'}, 1);
            addChild($$, $2);
        }
    |  '(' Exp ')'
        {
            $$ = $2;
        }
    |  NUM
        {
            $$ = makeNode(num, (union Value) {.num = $1}, 3);
        }
    |  CHARACTER
        {
            $$ = makeNode(character, (union Value) {.byte = $1}, 1);
        }
    |  LValue
        {
            $$ = $1;
        }
    |  IDENT '(' Arguments  ')'
        {
            union Value v;
            strcpy(v.str, $1);
            $$ = makeNode(ident_f, v, 2);
            addChild($$, $3);
        }
    ;
LValue:
       IDENT '[' Exp ']'
       {
            union Value v;
            strcpy(v.str, $1);
            $$ = makeNode(array, (union Value) {.num = 0}, 0);
            addChild($$, makeNode(ident, v, 2));
            Node* i = makeNode(index_, (union Value) {.num = 0}, 0);
            addChild($$, i);
            addChild(i, $3);
       }
    |  IDENT
        {
            union Value v;
            strcpy(v.str, $1);
            $$ = makeNode(ident, v, 2);
        }
    ;
Arguments:
       ListExp
        {
            $$ = makeNode(argument, (union Value) {.num = 0}, 0);
            addChild($$, $1);
        }
    |
        {
            $$ = NULL;
        }
    ;
ListExp:
       ListExp ',' Exp
        {
            addSibling($1, $3);
            $$ = $1;
        }
    |  Exp
        {
            $$ = $1;
        }
    ;
%%

int main(int argc, char *argv[]) {
    int opt;
    int treeFlag = 0;
    int symtabs = 0;
    char nasm_name[64] = "_anonymous.asm";

    struct option long_options[] = {
        {"symtabs", no_argument, 0, 's'},
        {"tree", no_argument, 0, 't'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "ths", long_options, NULL)) != -1) {
        switch (opt) {
            case 't':
                treeFlag = 1;
                break;
            case 'h':
                printf("User interface description:\n");
                printf("Options:\n");
                printf("  -t, --tree  Print the abstract tree to standard output\n");
                printf("  -s, --symtabs  Print symbol tables to standard output\n");
                printf("  -h, --help  Display this help and terminate execution\n");
                printf("Use:\n%s [-t] [-h] [-s] < [FILE.tpc]\nOR\n%s [-t] [-h] [-s] [FILE.tpc]\n", argv[0], argv[0]);
                return 0;
            case 's':
                symtabs = 1;
                break;
            default:
                fprintf(stderr, "invalid option\n");
                return 3;
        }
    }

    if(optind < argc) {
        yyin = fopen(argv[optind], "r");
        if(NULL == yyin) {
            fprintf(stderr, "ERROR : cant open the file %s\n", argv[optind]);
            return 3;
        }
        int len = strlen(argv[optind]);
        if(!strcmp(argv[optind]+len-4, ".tpc")) {
            strncpy(nasm_name, argv[optind], len - 3);
            nasm_name[len-3] = '\0';
            strcat(nasm_name, "asm");
        }else {
            fprintf(stderr, "ERROR : expected a file like 'file_name.tpc' but got 'file_name'\n");
            return 3;
        }
    }
    if(isatty(STDIN_FILENO) && optind == argc) {
        printf("Use:\n%s [-t] [-h] [-s] < [FILE.tpc]\nOR\n%s [-t] [-h] [-s] [FILE.tpc]\n", argv[0], argv[0]);
        return 3;
    }

    if (!yyparse()) {

        TabTabSymb *table = create_full_table(root);
        traduction(table, root, nasm_name);

        if(symtabs) {
            print_all_symb(table);
        }
        if (treeFlag) {
            printTree(root);
        }
        deleteTree(root);
        table_destroy(table);
        return 0;
    } else {
        return 1;
    }
}

void yyerror(char *msg) {
    fprintf(stderr, "l.%d c.%d : %s\n", lineno, nbChar, msg);
}
