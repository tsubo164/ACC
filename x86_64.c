#include <stdlib.h>
#include <string.h>
#include "x86_64.h"

static int att_syntax = 1;
static const char *oper_label;
static const char *oper_symbol;
static int  oper_label_id;
static int  oper_symbol_id;
static int  oper_register;
static int  oper_size;
static long oper_immediate;
static long oper_offset;
static int opecode_len = 0;
static int stack_offset = 8;

/*static*/ void inc_stack_pointer(int byte)
{
    stack_offset += byte;
}

/*static*/ void dec_stack_pointer(int byte)
{
    stack_offset -= byte;
}

void  reset_stack_offset(void)
{
    /* because of the return address is already pushed when a fuction starts
     * (rbp % 0x10) == 0x08 */
    stack_offset = 8;
}

int is_stack_aligned(void)
{
    return stack_offset % 16 == 0;
}

static const char register_name[][8] = {
    "a_",   "al",   "ax",   "eax",  "rax",
    "b_",   "bl",   "bx",   "ebx",  "rbx",
    "c_",   "cl",   "cx",   "ecx",  "rcx",
    "d_",   "dl",   "dx",   "edx",  "rdx",
    "di_",  "dil",  "di",   "edi",  "rdi",
    "si_",  "sil",  "si",   "esi",  "rsi",
    "bp_",  "bpl",  "bp",   "ebp",  "rbp",
    "sp_",  "spl",  "sp",   "esp",  "rsp",
    "r8_",  "r8b",  "r8w",  "r8d",  "r8",
    "r9_",  "r9b",  "r9w",  "r9d",  "r9",
    "r10_", "r10b", "r10w", "r10d", "r10",
    "r11_", "r11b", "r11w", "r11d", "r11",
    "r12_", "r12b", "r12w", "r12d", "r12",
    "r13_", "r13b", "r13w", "r13d", "r13",
    "r14_", "r14b", "r14w", "r14d", "r14",
    "r15_", "r15b", "r15w", "r15d", "r15"
};

static const enum operand_00 arg_reg_list[] = {DI__00, SI__00, D__00, C__00, R8__00, R9__00};

static const char directive[][8] = { "?", "byte", "word", "dword", "qword" };
static const char suffix_letter[] = {'?', 'b', 'w', 'l', 'q'};

static const char instruction_name[][8] = {
    "mov", "movsb", "movsw", "movsl", "movzb", "movzw",
    "add", "sub", "imul", "div", "idiv",
    "shl", "shr", "sar",
    "or", "xor", "and", "not", "cmp",

    "lea", "push", "pop",
    "call", "ret", "je", "jne", "jmp",
    "sete", "setne", "setl", "setg", "setle", "setge",

    "cltd", "cqto"
};

static int is_register(int oper)
{
    return oper >= A__00 && oper <= R15_00;
}

static int get_operand_size(int oper)
{
    if (is_register(oper))
        return oper % 5;
    return I0; /* '?' */
}

static void print_indent(FILE *fp)
{
    fprintf(fp, "    ");
}

static void print_separator(FILE *fp)
{
    int num = 6 - opecode_len;
    num = num > 1 ? num : 1;

    fprintf(fp, "%*s", num, " ");
}

static void print_opecode(FILE *fp, int op, int suffix)
{
    const char *inst = instruction_name[op];
    int len = strlen(inst);

    if (op < LEA_00) {
        fprintf(fp, "%s%c", inst, suffix_letter[suffix]);
        len++;
    }
    else if (op < JE_00) {
        fprintf(fp, "%s%c", inst, suffix_letter[I64]);
        len++;
    }
    else {
        fprintf(fp, "%s", inst);
    }

    if (op == PUSH_00)
        inc_stack_pointer(8);
    if (op == POP_00)
        dec_stack_pointer(8);

    opecode_len = len;
}

static void print_operand(FILE *fp, int oper, int suffix)
{
    const char *reg = NULL;

    if (is_register(oper)) {
        const int size = oper % 5;
        if (att_syntax)
            fprintf(fp, "%%");
        if (size == 0)
            reg = register_name[oper + oper_size];
        else
            reg = register_name[oper];
        fprintf(fp, "%s", reg);
        return;
    }

    switch (oper) {
    case OPR_IMM_00:
        if (att_syntax)
            fprintf(fp, "$");
        fprintf(fp, "%ld", oper_immediate);
        break;

    case OPR_MEM_00:
        reg = register_name[oper_register];
        if (att_syntax) {
            if (oper_offset == 0)
                fprintf(fp, "(%%%s)", reg);
            else
                fprintf(fp, "%ld(%%%s)", oper_offset, reg);
        } else {
            const char *direc = directive[suffix];
            if (oper_offset == 0)
                fprintf(fp, "%s ptr [%s]", direc, reg);
            else
                fprintf(fp, "%s ptr [%s%+ld]", direc, reg, oper_offset);
        }
        break;

    case OPR_SYM_00:
        if (att_syntax) {
            if (oper_symbol_id < 0)
                fprintf(fp, "_%s(%%rip)", oper_symbol);
            else
                fprintf(fp, "_%s_%d(%%rip)", oper_symbol, oper_symbol_id);
        } else {
            const char *direc = directive[suffix];
            if (oper_symbol_id > 0)
                fprintf(fp, "%s ptr [_%s_%d]", direc, oper_symbol, oper_symbol_id);
            else
                fprintf(fp, "%s ptr [_%s]", direc, oper_symbol);
        }
        break;

    case OPR_LBL_00:
        if (oper_label_id < 0)
            fprintf(fp, "_%s", oper_label);
        else
            fprintf(fp, "_%s_%d", oper_label, oper_label_id);
        break;

    default:
        break;
    }
}

enum operand_00 regi_00(enum operand_00 oper, enum operand_size size)
{
    if (is_register(oper) && oper % 5 == 0)
        return oper + size;
    else
        return A__00;
}

enum operand_00 arg_reg_00(int index, int size)
{
    if (index < 0 || index > 5)
        return A__00;

    return regi_00(arg_reg_list[index], size);
}

enum operand_00 imm_00(long val)
{
    oper_immediate = val;
    return OPR_IMM_00;
}

enum operand_00 mem_00(enum operand_00 reg, int offset)
{
    oper_register = reg;
    oper_offset = offset;
    return OPR_MEM_00;
}

enum operand_00 symb_00(const char *sym, int id)
{
    oper_symbol = sym;
    oper_symbol_id = id;
    return OPR_SYM_00;
}

enum operand_00 label_00(const char *label_name, int id)
{
    oper_label = label_name;
    oper_label_id = id;
    return OPR_LBL_00;
}

void set_operand_size(enum operand_size size)
{
    if (size >= I8 && size <= I64)
        oper_size = size;
    else
        oper_size = I0;
}

void code1_00(FILE *fp, enum opecode_00 op)
{
    print_indent(fp);
    print_opecode(fp, op, I64);

    fprintf(fp, "\n");
}

void code2_00(FILE *fp, enum opecode_00 op, enum operand_00 oper1)
{
    const int size1 = get_operand_size(oper1);
    int sfx;

    if (size1 > 0)
        sfx = size1;
    else
        sfx = oper_size;

    print_indent(fp);
    print_opecode(fp, op, sfx);

    print_separator(fp);
    print_operand(fp, oper1, sfx);

    fprintf(fp, "\n");
}

void code3_00(FILE *fp, enum opecode_00 op, enum operand_00 oper1, enum operand_00 oper2)
{
    const int size1 = get_operand_size(oper1);
    const int size2 = get_operand_size(oper2);
    int sfx;

    if (size1 > 0 || size2 > 0)
        sfx = size1 > size2 ? size1 : size2;
    else
        sfx = oper_size;

    print_indent(fp);
    print_opecode(fp, op, sfx);

    print_separator(fp);
    print_operand(fp, oper1, sfx);

    fprintf(fp, ", ");
    print_operand(fp, oper2, sfx);

    fprintf(fp, "\n");
}
