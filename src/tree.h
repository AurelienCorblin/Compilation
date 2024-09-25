/* tree.h */
#ifndef __TREE__
#define __TREE__

#include "symbol.h"

typedef enum {
  ident,
  ident_f,
  type_, 
  void_, 
  if_, 
  else_, 
  while_,
  return_, 
  or, 
  and, 
  eq, 
  order,
  addsub,
  addsub_u,
  divstar,
  num,
  character,
  return_type,
  heading,
  function,
  array,
  DeclFoncts,
  DeclVars,
  Prog,
  size,
  param,
  body,
  instruction,
  not,
  argument,
  Suiteinstr,
  index_,
  egale
} label_t;



union Value {
        char byte;
        char str[64];
        int num;
};

typedef struct Node {
    label_t label;
    struct Node *firstChild, *nextSibling;
    int lineno;
    int nodeType; // 0: noeud d'info, 1 : un byte, 2: une chaine de caract, 3: un entier.
    union Value value;
} Node;

Node *makeNode(label_t label, union Value v, int codeTypeNode);
void addSibling(Node *node, Node *sibling);
void addChild(Node *parent, Node *child);
void deleteTree(Node*node);
void printTree(Node *node);

#define FIRSTCHILD(node) node->firstChild
#define SECONDCHILD(node) node->firstChild->nextSibling
#define THIRDCHILD(node) node->firstChild->nextSibling->nextSibling

void fill_table(TabTabSymb *table, Node *node);
void fill_Vars(TabSymb *table, Node *node);
void fill_functions(TabTabSymb *table, Node *node);
void fill_params(TabSymb *table, Node *node);
void fill_funct_tab(TabTabSymb *table, Node *node);
void fill_Vars_func(TabFunct *table, Node *node);
TabTabSymb *create_full_table(Node *node);


void print_table(TabSymb *table);
void print_all_symb(TabTabSymb *table);

#endif