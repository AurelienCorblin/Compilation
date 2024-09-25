#include "tree.h"
#include "symbol.h"
#include "trad.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


extern int global_var_offset; // nb total octets necessaire pour les variables globale
int COMP_counter = 0;  // pour les expressions booléenne
int if_counter = 0;  // pour les branchements conditionnel
int lOOP_counter = 0;  // pour les boucles
int flag_last_return = 0; // pour check si on atteint la fin de la fct sans return
char param_reg[6][4] = {"rdi\0", "rsi\0", "rdx\0", "rcx\0", "r8\0", "r9\0"}; // nom des registres pour les 6 premiers param.


Symbol *check_id(TabTabSymb *table, TabFunct *local, Node *node) {
    Symbol *s = NULL;
    switch(node->label) {
        case ident:
            if(NULL != (s = func_get_var(local, node->value.str))) {
                if(s->c == VAR) return s;
                fprintf(stderr, "l.%d : %s is not a variable\n", node->lineno, node->value.str);
                exit(2);
            }
            if(NULL != (s = func_get_param(local, node->value.str))) {
                if(s->c == VAR) return s;
                fprintf(stderr, "l.%d : %s is not a variable\n", node->lineno, node->value.str);
                exit(2);
            }
            if(NULL != (s = table_get(table->global, node->value.str))) {
                if(s->c == VAR) return s;
                fprintf(stderr, "l.%d : %s is not a variable\n", node->lineno, node->value.str);
                exit(2);
            }
            fprintf(stderr, "l.%d : undeclared var %s\n", node->lineno, node->value.str);
            exit(2);
        case ident_f:
            if(NULL != (s = func_get_var(local, node->value.str))) {
                fprintf(stderr, "l.%d : %s is not a function\n", node->lineno, node->value.str);
                exit(2);
            }
            if(NULL != (s = func_get_param(local, node->value.str))) {
                fprintf(stderr, "l.%d : %s is not a function\n", node->lineno, node->value.str);
                exit(2);
            }
            if(NULL != (s = table_get(table->global, node->value.str))) {
                if(s->c == FUNC) return s;
                fprintf(stderr, "l.%d : %s is not a function\n", node->lineno, node->value.str);
                exit(2);
            }
            fprintf(stderr, "l.%d : undeclared function %s\n", node->lineno, node->value.str);
            exit(2);
        case array:
            if(NULL != (s = func_get_var(local, FIRSTCHILD(node)->value.str))) {
                if(s->c == TAB) return s;
                fprintf(stderr, "l.%d : %s is not an array\n", FIRSTCHILD(node)->lineno, FIRSTCHILD(node)->value.str);
                exit(2);
            }
            if(NULL != (s = func_get_param(local, FIRSTCHILD(node)->value.str))) {
                if(s->c == TAB) return s;
                fprintf(stderr, "l.%d : %s is not an array\n", FIRSTCHILD(node)->lineno, FIRSTCHILD(node)->value.str);
                exit(2);
            }
            if(NULL != (s = table_get(table->global, FIRSTCHILD(node)->value.str))) {
                if(s->c == TAB) return s;
                fprintf(stderr, "l.%d : %s is not an array\n", FIRSTCHILD(node)->lineno, FIRSTCHILD(node)->value.str);
                exit(2);
            }
            fprintf(stderr, "l.%d : undeclared array %s\n", FIRSTCHILD(node)->lineno, FIRSTCHILD(node)->value.str);
            exit(2);
        default:
            break;
    }
    return NULL;
}


Type check_expr_type(TabTabSymb *table, TabFunct *local, Node *node) {
    Type t;
    switch(node->label) {
        case num:
            return INT;
        case character:
            return CHAR;
        case ident:
            return check_id(table, local, node)->t;
        case array:
            check_expr_type(table, local, FIRSTCHILD(SECONDCHILD(node)));
            t = check_id(table, local, node)->t;
            return t;
        case ident_f:
            t = check_id(table, local, node)->t;
            if(t == VOID_) {
                fprintf(stderr, "l.%d ERROR : cant use type VOID in expression"
                " but %s is of type VOID\n", node->lineno, node->value.str);
                exit(2);
            }
            return t;
        case addsub: case divstar:case eq:case order:case and:case or:
            check_expr_type(table, local, FIRSTCHILD(node));
            check_expr_type(table, local, SECONDCHILD(node));
            return INT;
        case addsub_u:case not:
            check_expr_type(table, local, FIRSTCHILD(node));
            return INT;
        default:
            break;
    }
    return INT;
}


void write_read_var(TabTabSymb *table, TabFunct *local, Node *tree, FILE *out) {
    Symbol *s = NULL;
    s = check_id(table, local, tree);
    if(s->offset >= 0) {
        fprintf(out, "\tpush qword [global_var+%d]\n", s->offset);
    }else {
        fprintf(out, "\tpush qword [rbp%d]\n", s->offset);
    }
}


void write_read_tab(TabTabSymb *table, TabFunct *local, Node *tree, FILE *out) {
    Symbol *s = NULL;
    traduction_expression(table, local, FIRSTCHILD(SECONDCHILD(tree)), out);
    s = check_id(table, local, tree);
    if(s->offset >= 0) {
        fprintf(out, "\tpop rax\n\timul rax, 8\n\tpush qword [global_var+%d+rax]\n", s->offset);
    }else {
        if(func_get_var(local, s->id)) {
            fprintf(out, "\tpop rax\n\timul rax, 8\n\tpush qword [rbp%d+rax]\n", s->offset);
        }else {
            fprintf(out, "\tpop rax\n\timul rax, 8\n\tmov rdx, qword [rbp%d]\n"
            "\tpush qword [rdx+rax]\n", s->offset);
        }
    }
}


void calculate_argv(TabTabSymb *table, TabFunct *local, Symbol *p, Node *tree, FILE *out) {
    Symbol *s = NULL;
    switch(tree->label) {
        case ident:
            if(NULL == (s = func_get_var(local, tree->value.str))) {
                if(NULL == (s = func_get_param(local, tree->value.str))) {
                    if(NULL == (s = table_get(table->global, tree->value.str))) {
                        fprintf(stderr, "l.%d : undeclared var %s\n", tree->lineno, tree->value.str);
                        exit(2);
                    }
                }
            }
            if(s->c == VAR) {
                if(p->c == TAB) {
                    fprintf(stderr, "l.%d : expected an array where %s is a variable\n", tree->lineno, s->id);
                    exit(2);
                }
                if(s->offset >= 0) fprintf(out, "\tpush qword [global_var+%d]\n", s->offset);
                else {
                    fprintf(out, "\tpush qword [rbp%d]\n", s->offset);
                }
            }else if(s->c == TAB){
                if(p->c == VAR) {
                    fprintf(stderr, "l.%d : expected a variable where %s is an array \n", tree->lineno, s->id);
                    exit(2);
                }
                if(s->t != p->t) {
                    fprintf(stderr, "l.%d : %s is not the same type as expected\n", tree->lineno, s->id);
                    exit(2);
                }
                if(s->offset >= 0) fprintf(out, "\tmov rax, global_var\n\tadd rax, %d\n\tpush rax\n", s->offset);
                else {
                    fprintf(out, "\tmov rax, rbp\n\tadd rax, %d\n\tpush rax\n", s->offset);
                }
            }
            if(s->c == FUNC) {
                fprintf(stderr, "l.%d : calling %s without ( )\n", tree->lineno, tree->value.str);
                exit(2);
            }else if(s->t == INT && p->t == CHAR) {
                fprintf(stderr, "l.%d Warning :giving an int type token"
                " while expecting it to be a char\n", tree->lineno);
            }
            break;
        case array:
            if(NULL == (s = func_get_var(local, FIRSTCHILD(tree)->value.str))) {
                if(NULL == (s = func_get_param(local, FIRSTCHILD(tree)->value.str))) {
                    if(NULL == (s = table_get(table->global, FIRSTCHILD(tree)->value.str))) {
                        fprintf(stderr, "l.%d : undeclared array %s\n", tree->lineno, FIRSTCHILD(tree)->value.str);
                        exit(2);
                    }
                }
            }
            if(s->c != TAB) {
                fprintf(stderr, "l.%d : %s is not an array\n", tree->lineno, FIRSTCHILD(tree)->value.str);
                exit(2);
            }else {
                if(p->c == TAB) {
                    fprintf(stderr, "l.%d : expected an array but %s['EXP'] was found\n", tree->lineno, FIRSTCHILD(tree)->value.str);
                    exit(2);
                }
                if(s->t == INT && p->t == CHAR) {
                    fprintf(stderr, "l.%d Warning :giving an int type token"
                    " while expecting it to be a char\n", tree->lineno);
                }
                write_read_tab(table, local, tree, out);
            }
            break;
        case ident_f:
            if(NULL == (s = func_get_var(local, tree->value.str))) {
                if(NULL == (s = func_get_param(local, tree->value.str))) {
                    if(NULL == (s = table_get(table->global, tree->value.str))) {
                        fprintf(stderr, "l.%d : undeclared array %s\n", tree->lineno, tree->value.str);
                        exit(2);
                    }
                }
            }
            if(s->c != FUNC) {
                fprintf(stderr, "l.%d : %s is not a function\n", tree->lineno, tree->value.str);
                exit(2);
            }else {
                if(p->c == TAB) {
                    fprintf(stderr, "l.%d : expected an array but function %s was found\n", tree->lineno, tree->value.str);
                    exit(2);
                }if(s->t == VOID_) {
                    fprintf(stderr, "l.%d ERROR : cant use type VOID in expression"
                    " but %s is of type VOID\n", tree->lineno, tree->value.str);
                    exit(2);
                }
                if(s->t == INT && p->t == CHAR) {
                    fprintf(stderr, "l.%d Warning :giving an int type token"
                    " while expecting it to be a char\n", tree->lineno);
                }
                write_call(table, local, tree, out);
                fprintf(out, "\tpush rax\n");
            }
            break;
        default:
            if(check_expr_type(table, local, tree) == INT && p->t == CHAR) {
                fprintf(stderr, "l.%d Warning :giving an int type token to the function"
                " while expecting it to be a char\n", tree->lineno);
            }
            traduction_expression(table, local, tree, out);
            break;
    }
}


void write_arguments(TabTabSymb *table, TabFunct *local, TabFunct *f_called, Node *tree, FILE *out, int pos) {
    if(NULL == tree) {
        if(f_called->param_tab->size > pos) {
            fprintf(stderr, "l.%d : too few arguments were given to the function %s\n",
            table_get(table->global, f_called->name)->lineno, f_called->name);
            exit(2);
        }   
        return;
    }
    if(f_called->param_tab->size <= pos) {
        fprintf(stderr, "l.%d : too many arguments were given to the function %s\n", tree->lineno, f_called->name);
        exit(2);
    }
    write_arguments(table, local, f_called, tree->nextSibling, out, pos+1);
    Symbol *p = get_param_at(f_called, pos);
    calculate_argv(table, local, p, tree, out);
    if(pos < 6) {
        fprintf(out, "\tpop %s\n", param_reg[pos]);
    }
}


void write_call(TabTabSymb *table, TabFunct *local, Node *tree, FILE *out) {
    if(NULL == tree) {
        return;
    }
    TabFunct *f_called = get_f_table(table, tree->value.str);
    if(FIRSTCHILD(tree)) {
        write_arguments(table, local, f_called, FIRSTCHILD(FIRSTCHILD(tree)), out, 0);
    }
    int argc = f_called->param_tab->size;
    fprintf(out, "\tmov r11, rsp\n\tsub rsp, 8\n\tand rsp, -16\n\tmov qword [rsp], r11\n"
    "\tcall %s\n\tpop rsp\n\tadd rsp, %d\n", tree->value.str, argc - 6 > 0 ? (argc - 6)*8: 0);
}


void write_operation(TabTabSymb *table, TabFunct *local, Node *tree, FILE *out) {
    traduction_expression(table, local, FIRSTCHILD(tree),out);
    traduction_expression(table, local, SECONDCHILD(tree),out);

    fprintf(out,"\tpop rcx\n");
    fprintf(out,"\tpop rax\n");

    if(tree->value.byte == '+') {
        fprintf(out,"\tadd rax, rcx\n");
    }else if(tree->value.byte == '-') {
        fprintf(out,"\tsub rax, rcx\n");
    }else if(tree->value.byte == '*') {
        fprintf(out,"\timul rax, rcx\n");
    }else {
        fprintf(out,"\txor rdx, rdx\n\tidiv rcx\n");
        if(tree->value.byte == '/') {
            fprintf(out,"\tpush rax\n");
        }else if(tree->value.byte == '%') {
            fprintf(out,"\tpush rdx\n");
        }
        return;
    }
    fprintf(out,"\tpush rax\n");
}


void write_eq_order(TabTabSymb *table, TabFunct *local, Node *tree, FILE *out) {
    traduction_expression(table, local, FIRSTCHILD(tree),out);
    traduction_expression(table, local, SECONDCHILD(tree),out);
    fprintf(out,"\tpop rcx\n\tpop rax\n\tcmp rax, rcx\n");
    if(!strcmp(tree->value.str, "==")) fprintf(out, "\tje .COMP%d\n", COMP_counter);
    else if(!strcmp(tree->value.str, "!=")) fprintf(out, "\tjne .COMP%d\n", COMP_counter);
    else if(!strcmp(tree->value.str, "<")) fprintf(out, "\tjl .COMP%d\n", COMP_counter);
    else if(!strcmp(tree->value.str, ">")) fprintf(out, "\tjg .COMP%d\n", COMP_counter);
    else if(!strcmp(tree->value.str, ">=")) fprintf(out, "\tjge .COMP%d\n", COMP_counter);
    else if(!strcmp(tree->value.str, "<=")) fprintf(out, "\tjle .COMP%d\n", COMP_counter);
    fprintf(out, "\tpush 0\n\tjmp .COMP%d\n\t.COMP%d:\n\tpush 1\n\t.COMP%d:\n",
    COMP_counter+1, COMP_counter, COMP_counter+1);
    COMP_counter+=2;
}


void write_and_or(TabTabSymb *table, TabFunct *local, Node *tree, FILE *out) {
    traduction_expression(table, local, FIRSTCHILD(tree),out);
    traduction_expression(table, local, SECONDCHILD(tree),out);
    if(tree->label == and) {
        fprintf(out, "\tpop rcx\n\tpop rax\n\tcmp rax, 0\n\tje .COMP%d\n\tcmp rcx, 0\n\tje .COMP%d\n"
        "\tpush 1\n\tjmp .COMP%d\n\t.COMP%d:\n\tpush 0\n\t.COMP%d:\n", COMP_counter, COMP_counter,
        COMP_counter+1, COMP_counter, COMP_counter+1);
    } else {
        fprintf(out, "\tpop rcx\n\tpop rax\n\tcmp rax, 0\n\tjne .COMP%d\n\tcmp rcx, 0\n\tjne .COMP%d\n"
        "\tpush 0\n\tjmp .COMP%d\n\t.COMP%d:\n\tpush 1\n\t.COMP%d:\n", COMP_counter, COMP_counter,
        COMP_counter+1, COMP_counter, COMP_counter+1);
    }
    COMP_counter += 2;
}


void write_not(TabTabSymb *table, TabFunct *local, Node *tree, FILE *out) {
    traduction_expression(table, local, FIRSTCHILD(tree),out);
    fprintf(out, "\tpop rax\n\tcmp rax, 0\n\tje .COMP%d\n\tpush 0\n\tjmp .COMP%d\n\t.COMP%d:\n"
    "\tpush 1\n\t.COMP%d:\n", COMP_counter, COMP_counter+1, COMP_counter, COMP_counter+1);
    COMP_counter += 2;
}


void traduction_expression(TabTabSymb *table, TabFunct *local, Node *tree, FILE *out) {
    if(tree == NULL) {
        return;
    }
    switch (tree->label) {
        case ident:        
            write_read_var(table, local, tree, out);
            return;
        case array:
            write_read_tab(table, local, tree, out);
            return;
        case ident_f:
            write_call(table, local, tree, out);
            fprintf(out, "\tpush rax\n");
            return;
        case addsub_u:
            traduction_expression(table, local, FIRSTCHILD(tree),out);
            if(tree->value.byte == '-') {
                fprintf(out, "\tpop rax\n\timul rax, -1\n\tpush rax\n");
            }
            return;
        case num:
            fprintf(out,"\tpush %d\n", tree->value.num);
            return;
        case character:
            fprintf(out,"\tpush %d\n", tree->value.byte);
            return;
        case addsub:case divstar:
            write_operation(table, local, tree, out);
            return;
        case eq:case order:
            write_eq_order(table, local, tree, out);
            return;
        case and:case or:
            write_and_or(table, local, tree, out);
            return;
        case not:
            write_not(table, local, tree, out);
            return;
        default:
            return;
    }
}


void write_equal(TabTabSymb *table, TabFunct *local, Node *tree, FILE *out) {
    if(NULL == tree) {
        return;
    }
    Symbol *s = NULL;
    fprintf(out, "\tpop rcx\n");
    switch(tree->label) {
        case ident:        
            s = check_id(table, local, tree);
            if(s->offset >= 0) {
                fprintf(out, "\tmov qword [global_var+%d], rcx\n", s->offset);
            }else {
                fprintf(out, "\tmov qword [rbp%d], rcx\n", s->offset);
            }
            return;
        case array:
            s = check_id(table, local, tree);
            check_expr_type(table, local, FIRSTCHILD(SECONDCHILD(tree)));
            traduction_expression(table, local, FIRSTCHILD(SECONDCHILD(tree)), out);
            if(s->offset >= 0) {
                fprintf(out, "\tpop rax\n\timul rax, 8\n\tmov qword [global_var+%d+rax], rcx\n", s->offset);
            }else {
                if(func_get_var(local, s->id)) {
                    fprintf(out, "\tpop rax\n\timul rax, 8\n\tmov qword [rbp%d+rax], rcx\n", s->offset);
                }else {
                    fprintf(out, "\tpop rax\n\timul rax, 8\n\tmov rdx, qword [rbp%d]\n"
                    "\tmov qword [rdx+rax], rcx\n", s->offset);
                }
            }
            return;
        default:
            return;
    }
}


void write_return(TabTabSymb *table, TabFunct *local, Node *tree, FILE *out) {
    if(NULL == tree) {
        return;
    }
    Symbol *f = table_get(table->global, local->name);
    if(NULL == FIRSTCHILD(tree) && f->t != VOID_) {
        fprintf(stderr, "l.%d : ‘return’ with no value, in function (%s) returning non-void\n", tree->lineno, local->name);
        exit(2);
    }else if(FIRSTCHILD(tree)) {
        if(f->t == VOID_) {
            fprintf(stderr, "l.%d : ‘return’ with value, in function (%s) returning void\n", tree->lineno, local->name);
            exit(2);
        }
        if(check_expr_type(table, local, FIRSTCHILD(tree)) == INT && f->t == CHAR) {
            fprintf(stderr, "l.%d Warning :return an int type token"
            " while expecting it to be a char in function %s\n", tree->lineno, local->name);
        }
        traduction_expression(table, local, FIRSTCHILD(tree), out);
        fprintf(out, "\tpop rax\n");
        fprintf(out, "\tmov rsp, rbp\n\tpop rbp\n\tret\n");
    }else {
        fprintf(out, "\tmov rsp, rbp\n\tpop rbp\n\tret\n");
    }
}


void write_if(TabTabSymb *table, TabFunct *local, Node *tree, FILE *out) {
    int tmp_counter;
    check_expr_type(table, local, FIRSTCHILD(tree));
    tmp_counter = if_counter;
    if_counter = THIRDCHILD(tree) ? if_counter+2: if_counter+1;
    traduction_expression(table, local, FIRSTCHILD(tree), out);
    fprintf(out, "\tpop rax\n\tcmp rax, 0\n\tje .IF%d\n", tmp_counter);
    write_instructions(table, local, SECONDCHILD(tree), out);
    if(THIRDCHILD(tree)) {
        fprintf(out, "\tjmp .IF%d\n\t.IF%d:\n", tmp_counter+1, tmp_counter);
        write_instructions(table, local, THIRDCHILD(tree), out);
        fprintf(out, "\t.IF%d:\n", tmp_counter+1);
    }else fprintf(out, "\t.IF%d:\n", tmp_counter);
}


void write_loop(TabTabSymb *table, TabFunct *local, Node *tree, FILE *out) {
    int tmp_counter;
    check_expr_type(table, local, FIRSTCHILD(tree));
    tmp_counter = lOOP_counter;
    lOOP_counter += 2;
    fprintf(out, "\tjmp .L%d\n\t.L%d:\n", tmp_counter, tmp_counter+1);
    write_instructions(table, local, SECONDCHILD(tree), out);
    fprintf(out, "\t.L%d:\n", tmp_counter);
    traduction_expression(table, local, FIRSTCHILD(tree), out);
    fprintf(out, "\tpop rax\n\tcmp rax, 0\n\tjne .L%d\n", tmp_counter+1);
}


void write_instructions(TabTabSymb *table, TabFunct *local, Node *tree, FILE *out) {
    if(NULL == tree) {
        return;
    }
    flag_last_return = 0;
    switch(tree->label) {
        case ident_f:
            check_id(table, local, tree);
            write_call(table, local, tree, out);
            return;
        case egale:
            if(check_expr_type(table, local, SECONDCHILD(tree)) == INT && 
            check_id(table, local, FIRSTCHILD(tree))->t == CHAR) {
                fprintf(stderr, "l.%d Warning :giving an int type token"
                " while expecting it to be a char\n", tree->lineno);
            }
            traduction_expression(table, local, SECONDCHILD(tree), out);
            write_equal(table, local, FIRSTCHILD(tree), out);
            return;
        case if_:
            write_if(table, local, tree, out);
            return;
        case while_:
            write_loop(table, local, tree, out);
            return;
        case return_:
            write_return(table, local, tree, out);
            flag_last_return = 1;
            return;
        case instruction:
            for(Node *tmp = FIRSTCHILD(tree); tmp; tmp = tmp->nextSibling) {
                write_instructions(table, local, tmp, out);
            }
            return;
        default:
            return;
    }
}


void copy_argv(TabFunct *local, FILE *out) {
    int nb = local->param_tab->size;
    if(nb <= 6) {
        for(int i = 0; i < nb; i++) {
            fprintf(out,"\tpush %s\n", param_reg[i]);
        }
    }else {
        for(int i = 0; i < 6; i++) {
            fprintf(out,"\tpush %s\n", param_reg[i]);
        }
        int offset = 0;
        for(int i = 6; i < nb; i++) {
            fprintf(out, "\tmov rax, qword [rbp+16]\n\tmov rax, qword [rax+%d]\n"
            "\tpush rax\n", offset);
            offset += 8;
        }
    }
    
}


void write_local_vars(TabFunct *local, FILE *out) {
    copy_argv(local, out);
    fprintf(out, "\tsub rsp, %d\n", local->size_local_var);
}


void write_body(TabTabSymb *table, TabFunct *local, Node *tree, FILE *out) {
    if(NULL == tree) {
        return;
    }
    flag_last_return = 0;
    fprintf(out, "%s:\n\tpush rbp\n\tmov rbp, rsp\n", local->name);
    write_local_vars(local, out);
    for(Node *tmp = SECONDCHILD(tree); tmp; tmp = tmp->nextSibling) {
        write_instructions(table, local, tmp, out);
    }
    if(!flag_last_return && table_get(table->global, local->name)->t != VOID_) {
        fprintf(stderr, "l.%d Warning : reaches end of function without return from the non-void type function %s\n", tree->lineno, local->name);
    }
    if(!flag_last_return) fprintf(out, "\tmov rsp, rbp\n\tpop rbp\n\tret\n");
}


void write_asm(TabTabSymb *table, Node *tree, FILE *out) {
    if(NULL == tree) {
        return;
    }
    TabFunct *local = get_f_table(table, SECONDCHILD(FIRSTCHILD(tree))->value.str);
    write_body(table, local, SECONDCHILD(tree), out);
    
    write_asm(table, tree->nextSibling, out);
}


void traduction(TabTabSymb *table, Node *tree, char *nasm_name) {
    FILE *out;
    if(NULL == (out = fopen(nasm_name, "w+"))) {
        fprintf(stderr, "failed attempt to open or create the '%s' file (fopen with 'w+')\n", nasm_name);
        exit(3);
    }
    Symbol *s;
    if(NULL == (s = table_get(table->global, "main")) || s->c != FUNC) {
        fprintf(stderr, "ERROR : main function not found\n");
        exit(2);
    }
    if(s->t != INT) {
        fprintf(stderr, "ERROR : main return must be int\n");
        exit(2);
    }
    // table->size -4 pour ne pas compter les 4 fonctions par default qui n ont pas d'offset(getint, putint ...)
    fprintf(out, "section .bss\nglobal_var: resq %d\n", (global_var_offset)/8 - (table->size-4));
    fprintf(out, "\nsection .text\nglobal _start\n");
    fprintf(out, "%s\n%s\n%s\n%s\n", GETINT, PUTINT, GETCHAR, PUTCHAR);
    write_asm(table, FIRSTCHILD(SECONDCHILD(tree)), out);
    fprintf(out, "\n\n_start:\n\tcall main\n\tmov rdi, rax\n\tmov rax, 60\n\tsyscall\n");
    fclose(out);
}