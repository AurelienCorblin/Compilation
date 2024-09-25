#include "symbol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void init_tablesymb(TabSymb *table) {
    if(NULL == (table->tab = (Symbol *) malloc(sizeof(Symbol) * 5))) {
        printf("malloc failure");
        exit(3);
    }
    table->size = 0;
    table->capacity = 5;
}

void init_tab_funct(TabFunct *tables, char *name) {
    strcpy(tables->name, name);
    tables->param_tab = (TabSymb *)malloc(sizeof(TabSymb));
    tables->vars_tab = (TabSymb *)malloc(sizeof(TabSymb));
    if (tables->param_tab == NULL || tables->vars_tab == NULL) {
        printf("malloc failure");
        exit(3);
    }
    init_tablesymb(tables->param_tab);
    init_tablesymb(tables->vars_tab);
}


void init_tabtab(TabTabSymb *tables) {
    if(NULL == (tables->tab = (TabFunct *) malloc(sizeof(TabFunct) * 5))) {
        printf("malloc failure");
        exit(3);
    }
    tables->size = 0;
    tables->capacity = 5;

    tables->global = (TabSymb *)malloc(sizeof(TabSymb));
    if (tables->global == NULL) {
        printf("malloc failure");
        exit(3);
    }
    init_tablesymb(tables->global);
}


void resize_tabfunct(TabTabSymb *tables) {
    tables->capacity *= 2;
    if ((tables->tab = realloc(tables->tab, sizeof(TabFunct) * (tables->capacity))) == NULL){
        printf("realloc failure");
        exit(3);
    }
}

void put_tabfunct(TabTabSymb *tables, TabFunct f) {
    if(tables->size == tables->capacity) {
        resize_tabfunct(tables);
    }
    tables->tab[tables->size++] = f;
}

Symbol* func_get_param(TabFunct *tables, char *key) {
    TabSymb *table = tables->param_tab;
    for(int i = 0; i < table->size; i++){
        if(strcmp(table->tab[i].id, key) == 0){
            return &(table->tab[i]);
        }
    }
    return NULL;
}

Symbol* func_get_var(TabFunct *tables, char *key) {
    return table_get(tables->vars_tab, key);
}

void put_vars(TabFunct *tables, Symbol value) {
    if(func_get_param(tables, value.id) == NULL) {
        table_put(tables->vars_tab, value);
        return;
    }
    fprintf(stderr, "l.%d : redeclaration of %s\n", value.lineno, value.id);
    exit(2);
}


void table_put(TabSymb *table, Symbol value){
    if(table_get(table, value.id) == NULL) {
        if (table->size == table->capacity){
            resize_table(table);
        }
        table->tab[table->size++] = value;
        return;
    }
    fprintf(stderr, "l.%d : redeclaration of %s\n", value.lineno, value.id);
    exit(2);
}

Symbol* table_get(TabSymb *table, char *key){
    for(int i = 0; i < table->size; i++){
        if(strcmp(table->tab[i].id, key) == 0){
            return &(table->tab[i]);
        }
    }
    return NULL;
}


Symbol* get_param_at(TabFunct *table, int i){
    TabSymb *tmp = table->param_tab;
    if(i < tmp->size) {
        return &(tmp->tab[i]);
    }
    return NULL;
}


TabFunct *get_f_table(TabTabSymb *table, char *key) {
    for(int i = 0; i < table->size; i++){
        if(strcmp(table->tab[i].name, key) == 0){
            return &(table->tab[i]);
        }
    }
    return NULL;
}

void resize_table(TabSymb *table){
    table->capacity *= 2;
    if ((table->tab = realloc(table->tab, sizeof(Symbol) * (table->capacity))) == NULL){
        printf("realloc failure");
        exit(3);
    }
}

Symbol new_symbol(char *id, Type t, Classe c, int lineno, int offset) {
    Symbol new;
    strcpy(new.id, id);
    new.t = t;
    new.c = c;
    new.lineno = lineno;
    new.offset = offset;

    return new;
}


void table_destroy(TabTabSymb *table) {
    for(int i = 0; i < table->size; i++) {
        free(table->tab[i].param_tab->tab);
        free(table->tab[i].param_tab);
        free(table->tab[i].vars_tab->tab);
        free(table->tab[i].vars_tab);
    }
    free(table->global->tab);
    free(table->global);
    free(table->tab);
    free(table);
    table = NULL;
}