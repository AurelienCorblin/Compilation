/* tree.c */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "tree.h"
extern int lineno;       /* from lexer */

static const char *StringFromLabel[] = {
  "ident",
  "type_", 
  "void_", 
  "if_", 
  "else_", 
  "while_",
  "return_", 
  "or", 
  "and", 
  "eq", 
  "order",
  "addsub",
  "divstar",
  "num",
  "character",
  "return_type",
  "heading",
  "function",
  "array",
  "DeclFoncts",
  "DeclVars",
  "Prog",
  "size",
  "param",
  "body",
  "instructions",
  "not",
  "argument",
  "Suiteinstr",
  "index",
  "egale"
};

Node *makeNode(label_t label, union Value v, int codeTypeNode) {
  Node *node = malloc(sizeof(Node));
  if (!node) {
    printf("Run out of memory\n");
    exit(2);
  }
  node->label = label;
  node-> firstChild = node->nextSibling = NULL;
  node->lineno=lineno;
  node->value = v;
  node->nodeType = codeTypeNode;
  return node;
}

void addSibling(Node *node, Node *sibling) {
  Node *curr = node;
  while (curr->nextSibling != NULL) {
    curr = curr->nextSibling;
  }
  curr->nextSibling = sibling;
}

void addChild(Node *parent, Node *child) {
  if (parent->firstChild == NULL) {
    parent->firstChild = child;
  }
  else {
    addSibling(parent->firstChild, child);
  }
}

void deleteTree(Node *node) {
  if (node->firstChild) {
    deleteTree(node->firstChild);
  }
  if (node->nextSibling) {
    deleteTree(node->nextSibling);
  }
  free(node);
}

void printTree(Node *node) {
  static bool rightmost[128]; // tells if node is rightmost sibling
  static int depth = 0;       // depth of current node
  for (int i = 1; i < depth; i++) { // 2502 = vertical line
    printf(rightmost[i] ? "    " : "\u2502   ");
  }
  if (depth > 0) { // 2514 = L form; 2500 = horizontal line; 251c = vertical line and right horiz 
    printf(rightmost[depth] ? "\u2514\u2500\u2500 " : "\u251c\u2500\u2500 ");
  }
  if(node->nodeType == 1) {
    printf("%c", node->value.byte);
    printf("\n");
  }
  else if(node->nodeType == 2) {
    printf("%s", node->value.str);
    printf("\n");
  }
  else if(node->nodeType == 3) {
    printf("%d", node->value.num);
    printf("\n");
  }
  else {
    printf("%s", StringFromLabel[node->label]);
    printf("\n");
  }
  depth++;
  for (Node *child = node->firstChild; child != NULL; child = child->nextSibling) {
    rightmost[depth] = (child->nextSibling) ? false : true;
    printTree(child);
  }
  depth--;
}
