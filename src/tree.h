/* tree.h */

typedef enum {
  ident,
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
