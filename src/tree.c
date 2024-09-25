/* tree.c */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"
#include "symbol.h"
extern int lineno;       /* from lexer */
int function_offset = 0;
int global_var_offset = 0;

static const char *StringFromLabel[] = {
  "ident",
  "ident_f",
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
  "addsub_unary",
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
    exit(3);
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
    if(node->value.byte == 10) {
      printf("%s : \\n",StringFromLabel[node->label]);
      printf("\n");
    }
    else if(node->value.byte == 9) {
      printf("%s : \\t",StringFromLabel[node->label]);
      printf("\n");
    }
    else {
      printf("%s : %c",StringFromLabel[node->label], node->value.byte);
      printf("\n");
    }
  }
  else if(node->nodeType == 2) {
    printf("%s : %s",StringFromLabel[node->label], node->value.str);
    printf("\n");
  }
  else if(node->nodeType == 3) {
    printf("%s : %d",StringFromLabel[node->label], node->value.num);
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

/*******************************************
*       function for symbol table :        *
*                                          *
*******************************************/

void add_default_func(TabTabSymb *table) {
  while(table->size + 4 >= table->capacity) {
    resize_tabfunct(table);
  }
  init_tab_funct(&(table->tab[table->size]), "putchar");
  table_put(table->tab[table->size].param_tab, new_symbol("c", CHAR, VAR, 0, -8)); 
  table->size++;
  
  init_tab_funct(&(table->tab[table->size]), "putint");
  table_put(table->tab[table->size].param_tab, new_symbol("n", INT, VAR, 0, -8));
  table->size++;

  init_tab_funct(&(table->tab[table->size++]), "getint");
  init_tab_funct(&(table->tab[table->size++]), "getchar");
}

void fill_table(TabTabSymb *table, Node *node) {
    if(NULL == node) {
      return;
    }
    if(node->label == DeclVars) {
        // ajout des fonctions par default, ne pouvant pas etre redefinie
        table_put(table->global, new_symbol("getint", INT, FUNC, 0, 0));
        table_put(table->global, new_symbol("getchar", CHAR, FUNC, 0, 0));
        table_put(table->global, new_symbol("putchar", VOID_, FUNC, 0, 0));
        table_put(table->global, new_symbol("putint", VOID_, FUNC, 0, 0));
        
        fill_Vars(table->global, FIRSTCHILD(node));
    }else if(node->label == DeclFoncts) {
        fill_functions(table, FIRSTCHILD(node));

        add_default_func(table);
    }

    fill_table(table, node->nextSibling);
}


void fill_Vars(TabSymb *table, Node *node) {
	if(NULL == node) {
		return;
	}
	if(node->label == type_) {
		Node *tmp = FIRSTCHILD(node);
		Type t;
		if(strcmp(node->value.str, "int") == 0) {
		t = INT;
		}
		else if(strcmp(node->value.str, "char") == 0) {
		t = CHAR;
		}
		while(tmp) {
			if(NULL == FIRSTCHILD(tmp)) {
				table_put(table, new_symbol(tmp->value.str, t, VAR, tmp->lineno, global_var_offset));
				global_var_offset += 8;
			}
			else {
				table_put(table, new_symbol(FIRSTCHILD(tmp)->value.str, t, TAB, tmp->lineno, global_var_offset));
				if(SECONDCHILD(tmp)->value.num <= 0) {
					fprintf(stderr, "l.%d : size of array %s is null or negative\n", tmp->lineno
					, SECONDCHILD(tmp)->value.str);
					exit(2);
				}
				global_var_offset += 8 * SECONDCHILD(tmp)->value.num;
			}
			tmp = tmp->nextSibling;
		}
	}
	fill_Vars(table, node->nextSibling);
}


void fill_functions(TabTabSymb *table, Node *node) {
	if(NULL == node) {
		return;
	}
	if(node->label == function) {
		Type t;
		if(FIRSTCHILD(FIRSTCHILD(node))->label == void_) {
			t = VOID_;
		}
		else if(strcmp(FIRSTCHILD(FIRSTCHILD(node))->value.str, "int") == 0){
			t = INT;
		}
		else {
			t = CHAR;
		}
		Node *current = SECONDCHILD(FIRSTCHILD(node));
		table_put(table->global, new_symbol(current->value.str, t, FUNC, current->lineno, global_var_offset));
		global_var_offset += 8;

		fill_funct_tab(table, node);
	}
	fill_functions(table, node->nextSibling);
}


void fill_params(TabSymb *table, Node *node) {
	if(NULL == node || void_ == node->label) {
		return;
	}
	if(node->label == type_) {
		Node *tmp = FIRSTCHILD(node);
		Type t;
		if(strcmp(node->value.str, "int") == 0) {
			t = INT;
		}
		else if(strcmp(node->value.str, "char") == 0) {
			t = CHAR;
		}
		function_offset -= 8;
		if(NULL == FIRSTCHILD(tmp)) {
			table_put(table, new_symbol(tmp->value.str, t, VAR, tmp->lineno, function_offset));
		}
		else {
			table_put(table, new_symbol(FIRSTCHILD(tmp)->value.str, t, TAB, tmp->lineno, function_offset));
		}
		tmp = tmp->nextSibling;
	}
	fill_params(table, node->nextSibling);
}


void fill_Vars_func(TabFunct *table, Node *node) {
	if(NULL == node) {
		return;
	}
	if(node->label == type_) {
		Node *tmp = FIRSTCHILD(node);
		Type t;
		if(strcmp(node->value.str, "int") == 0) {
			t = INT;
		}
		else if(strcmp(node->value.str, "char") == 0) {
			t = CHAR;
		}
		while(tmp) {
			if(NULL == FIRSTCHILD(tmp)) {
				function_offset -= 8;
				put_vars(table, new_symbol(tmp->value.str, t, VAR, tmp->lineno, function_offset));
				
			}
			else {
				function_offset -= 8 * SECONDCHILD(tmp)->value.num;
				put_vars(table, new_symbol(FIRSTCHILD(tmp)->value.str, t, TAB, tmp->lineno, function_offset));
				if(SECONDCHILD(tmp)->value.num <= 0) {
					fprintf(stderr, "l.%d : size of array %s is null or negative\n", tmp->lineno
					, SECONDCHILD(tmp)->value.str);
					exit(2);
				}
			}
			tmp = tmp->nextSibling;
		}
	}
	fill_Vars_func(table, node->nextSibling);
}


void fill_funct_tab(TabTabSymb *table, Node *node) {
	if(table->size == table->capacity) {
		resize_tabfunct(table);
	}
	init_tab_funct(&(table->tab[table->size]), SECONDCHILD(FIRSTCHILD(node))->value.str);
	function_offset = 0;
	fill_params(table->tab[table->size].param_tab, FIRSTCHILD(THIRDCHILD(FIRSTCHILD(node))));
	int size_param = function_offset;
	fill_Vars_func(&(table->tab[table->size]), FIRSTCHILD(FIRSTCHILD(SECONDCHILD(node))));
	table->tab[table->size].size_local_var = (function_offset - size_param) * (-1);

	table->size++;
}


TabTabSymb *create_full_table(Node *node) {
	TabTabSymb *table = NULL;
	if(NULL == (table = (TabTabSymb *) malloc(sizeof(TabTabSymb)))) {
		printf("malloc failure\n");
		exit(3);
	}
	init_tabtab(table);
	fill_table(table, node->firstChild);
	return table;
}


void print_table(TabSymb *table) {
	if(table == NULL) return;
	for(int i = 0; i < table->size; i++) {
		printf("line: %d; offset: %d; type: %d; classe: %d; name: %s\n", table->tab[i].lineno,
		table->tab[i].offset, table->tab[i].t, table->tab[i].c, table->tab[i].id);
	}
}

void print_all_symb(TabTabSymb *table) {
	printf("Help:\ntype -> 0:int, 1:char, 2:void\nclasse -> 0:tab, 1:var, 2:func\n");
	printf("\ntable of global vars :\n");
	printf("-------------------------------\n");
	print_table(table->global);
	printf("-------------------------------\n");
	printf("\n\n");
	for(int i = 0; i < table->size; i++) {
		printf("table of function : %s\n", table->tab[i].name);
		printf("-------------------------------\n");
		printf("\nPARAM\n");
		print_table(table->tab[i].param_tab);
		printf("\nVARS\n");
		print_table(table->tab[i].vars_tab);
		printf("-------------------------------\n\n");
	}
}