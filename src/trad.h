#ifndef __TRAD__
#define __TRAD__

#include "tree.h"
#include "symbol.h"
#include <stdio.h>

void traduction(TabTabSymb *table, Node *tree, char *nasm_name);

void traduction_expression(TabTabSymb *table, TabFunct *local, Node *tree, FILE *out);

void write_instructions(TabTabSymb *table, TabFunct *local, Node *tree, FILE *out);
void write_call(TabTabSymb *table, TabFunct *local, Node *tree, FILE *out);

#define GETINT "getint:\n\tpush rbp\n\tmov rbp, rsp\n\tmov ebx, 0\n\tmov r9, 1\n\tsub rsp, 8\n\tmov rax, 0\n\tmov rdi, 0\
\n\tmov rsi, rsp\n\tmov rdx, 1\n\tsyscall\n\tpop rax\n\tcmp rax, '-'\n\tjne .firstcheck\n\tmov r9, -1\
\n\t.firstcheck:\n\t\tcmp rax, '0'\n\t\tjl .error\n\t\tcmp rax, '9'\n\t\tjg .error\n\t\tsub rax, 48\
\n\t\timul ebx, 10\n\t\tadd ebx, eax\n\t\tjmp .read\n\t.readfirst:\n\t\tsub rsp, 8\n\t\tmov rax, 0\
\n\t\tmov rdi, 0\n\t\tmov rsi, rsp\n\t\tmov rdx, 1\n\t\tsyscall\n\t\tpop rax\n\t\tcmp rax, '0'\n\t\tjl .error\
\n\t\tcmp rax, '9'\n\t\tjg .error\n\t\tsub rax, 48\n\t\timul ebx, 10\n\t\tadd ebx, eax\n\t.read:\n\t\tsub rsp, 8\
\n\t\tmov rax, 0\n\t\tmov rdi, 0\n\t\tmov rsi, rsp\n\t\tmov rdx, 1\n\t\tsyscall\n\t\tpop rax\n\t\tcmp rax, '0'\
\n\t\tjl .end\n\t\tcmp rax, '9'\n\t\tjg .end\n\t\tsub rax, 48\n\t\timul ebx, 10\n\t\tadd ebx, eax\n\t\tjmp .read\
\n\t.end:\n\t\tmov eax, ebx\n\t\timul rax, r9\n\t\tmov rsp, rbp\n\t\tpop rbp\n\t\tret\n\t.error:\n\t\tmov rdi, 5\
\n\t\tmov rax, 60\n\t\tsyscall\n"

#define PUTINT "putint:\n\tpush rbp\n\tmov rbp,rsp\n\tmov r10, 0\n\tmov r8,10\n\tmov r9,0\n\tmov rax,rdi\n\tmov rbx,10\n\tsub rsp,10\n\tcmp rax, 0\
\n\tjge calc_p\n\timul rax, -1\n\tmov r10, 1\n\tcalc_p:\n\tmov rdx,0\n\tidiv rbx\n\tadd rdx,48\n\tmov [rsp + r8 -1],dl\n\tinc r9\n\tdec r8\n\tcmp rax,0\
\n\tje print_p\n\tjmp calc_p\n\tprint_p:\n\tcmp r10, 1\n\tjne .next_print_p\n\tsub rsp, 1\n\tmov qword [rsp], '-'\n\tmov rax,1\n\tmov rdi,1\n\tmov rsi,rsp\
\n\tmov rdx,1\n\tsyscall\n\tadd rsp, 1\n\t.next_print_p:\n\tmov rax,1\n\tmov rdi,1\n\tmov rsi,rsp\n\tadd rsi,r8\n\tmov rdx,r9\n\tsyscall\n\tmov rax,1\
\n\tmov rdi,1\n\tpush 10\n\tmov rsi,rsp\n\tmov rdx,1\n\tsyscall\n\tmov rsp,rbp\n\tpop rbp\n\tret\n"

#define PUTCHAR "putchar:\n\tpush rbp\n\tmov rbp, rsp\n\tpush rdi\n\tmov rsi, rsp\n\tmov rax, 1\n\tmov rdi, 1\n\tmov rdx, 1\n\tsyscall\
\n\tmov rax,1\n\tmov rdi,1\n\tpush 10\n\tmov rsi,rsp\n\tmov rdx,1\n\tsyscall\n\tmov rsp , rbp\n\tpop rbp\n\tret\n"

#define GETCHAR "getchar:\n\tpush rbp\n\tmov rbp, rsp\n\tsub rsp, 8\n\tmov rax, 0\n\tmov rdi, 0\n\tmov rsi, rsp\n\tmov rdx, 1\
\n\tsyscall\n\tpop rax\n\tmov rsp , rbp\n\tpop rbp\n\tret\n"

#endif