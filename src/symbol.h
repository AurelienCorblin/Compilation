#ifndef __SYMBOL__
#define __SYMBOL__

#include <stdbool.h>


typedef enum{INT, CHAR, VOID_}Type;
typedef enum{TAB, VAR, FUNC}Classe;


typedef struct{
    char id[64];
    Type t;
    Classe c;
    int lineno;
    int offset; //ignorer pour les symboles representant une fct
}Symbol;

typedef struct {
    Symbol *tab;
    int size;
    int capacity;
}TabSymb;

typedef struct {
    char name[64];
    int size_local_var;
    TabSymb *param_tab;
    TabSymb *vars_tab;
} TabFunct;

typedef struct {
    TabSymb *global;
    TabFunct *tab;
    int size;
    int capacity;
}TabTabSymb;


void init_tablesymb(TabSymb *table);

void init_tab_funct(TabFunct *tables, char *name);

void init_tabtab(TabTabSymb *tables);

void resize_tabfunct(TabTabSymb *tables);

void put_tabfunct(TabTabSymb *tables, TabFunct f);

Symbol* func_get_param(TabFunct *tables, char *key);

Symbol* func_get_var(TabFunct *tables, char *key);

TabFunct *get_f_table(TabTabSymb *table, char *key);

void put_vars(TabFunct *tables, Symbol value);

void table_put(TabSymb *table, Symbol value);

Symbol* table_get(TabSymb *table, char *key);

void resize_table(TabSymb *table);

Symbol new_symbol(char *id, Type t, Classe c, int lineno, int offset);

Symbol* get_param_at(TabFunct *table, int i);

void table_destroy(TabTabSymb *table);

#endif