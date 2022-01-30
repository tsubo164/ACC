#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include "gen_x64.h"
#include "type.h"
#include "esc_seq.h"

static int att_syntax = 1;
static const char *operand_label;
static const char *operand_symbol;
static int  operand_label_id;
static int  operand_symbol_id;
static int  operand_register;
static long operand_immediate;
static long operand_offset;
static int  opecode_mnem = 0;
static int  opecode_len = 0;
/* because of the return address is already pushed when a fuction starts
 * (rbp % 0x10) == 0x08 */
static int stack_offset = 8;

enum operand_size {
    I0, I8, I16, I32, I64
};

enum operand {
    A_,   AL,   AX,   EAX,  RAX,
    B_,   BL,   BX,   EBX,  RBX,
    C_,   CL,   CX,   ECX,  RCX,
    D_,   DL,   DX,   EDX,  RDX,
    DI_,  DIL,  DI,   EDI,  RDI,
    SI_,  SIL,  SI,   ESI,  RSI,
    BP_,  BPL,  BP,   EBP,  RBP,
    SP_,  SPL,  SP,   ESP,  RSP,
    R8_,  R8B,  R8W,  R8D,  R8,
    R9_,  R9B,  R9W,  R9D,  R9,
    R10_, R10B, R10W, R10D, R10,
    R11_, R11B, R11W, R11D, R11,
    R12_, R12B, R12W, R12D, R12,
    R13_, R13B, R13W, R13D, R13,
    R14_, R14B, R14W, R14D, R14,
    R15_, R15B, R15W, R15D, R15,
    OPR_IMM,
    OPR_MEM,
    OPR_LBL,
    OPR_SYM
};

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

static const enum operand arg_reg_list[] = {DI_, SI_, D_, C_, R8_, R9_};

static int is_register(int oper)
{
    return oper >= A_ && oper <= R15;
}

enum opecode {
    MOV, MOVSB, MOVSW, MOVSL, MOVZB, MOVZW,
    ADD, SUB, IMUL, DIV, IDIV,
    SHL, SHR, SAR,
    OR, XOR, AND, NOT, CMP,
    LEA, PUSH, POP,
    CALL, RET, JE, JNE, JMP,
    SETE, SETNE, SETL, SETG, SETLE, SETGE,
    CLTD, CQTO
};

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

static const char directive[][8] =  {"?", "byte", "word", "dword", "qword"};
static const char data_name[][8] =  {"?", "byte", "word", "long",  "quad"};
static const char suffix_letter[] = {'?', 'b',    'w',    'l',     'q'};

static void inc_stack_pointer(int byte)
{
    stack_offset += byte;
}

static void dec_stack_pointer(int byte)
{
    stack_offset -= byte;
}

static int is_stack_aligned(void)
{
    return stack_offset % 16 == 0;
}

static int get_operand_size(int oper)
{
    if (is_register(oper))
        return oper % 5;
    /* '?' */
    return I0;
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

    if (op < LEA) {
        fprintf(fp, "%s%c", inst, suffix_letter[suffix]);
        len++;
    }
    else if (op < JE) {
        fprintf(fp, "%s%c", inst, suffix_letter[I64]);
        len++;
    }
    else {
        fprintf(fp, "%s", inst);
    }

    if (op == PUSH)
        inc_stack_pointer(8);
    if (op == POP)
        dec_stack_pointer(8);

    if (op == CALL)
        opecode_mnem = CALL;
    else
        opecode_mnem = 0;

    opecode_len = len;
}

static void print_operand(FILE *fp, int oper, int suffix)
{
    const char *reg = NULL;

    if (is_register(oper)) {
        const int size = oper % 5;
        if (size == 0)
            reg = register_name[oper + suffix];
        else
            reg = register_name[oper];

        if (att_syntax)
            fprintf(fp, "%%");
        fprintf(fp, "%s", reg);
        return;
    }

    switch (oper) {
    case OPR_IMM:
        if (att_syntax)
            fprintf(fp, "$");
        fprintf(fp, "%ld", operand_immediate);
        break;

    case OPR_MEM:
        reg = register_name[operand_register];
        if (att_syntax) {
            if (opecode_mnem == CALL)
                /* indirect jump */
                fprintf(fp, "*");

            if (operand_offset == 0)
                fprintf(fp, "(%%%s)", reg);
            else
                fprintf(fp, "%ld(%%%s)", operand_offset, reg);
        } else {
            const char *direc = directive[suffix];
            if (operand_offset == 0)
                fprintf(fp, "%s ptr [%s]", direc, reg);
            else
                fprintf(fp, "%s ptr [%s%+ld]", direc, reg, operand_offset);
        }
        break;

    case OPR_SYM:
        if (att_syntax) {
            if (opecode_mnem == CALL)
                /* indirect jump */
                fprintf(fp, "*");

            if (operand_symbol_id < 0)
                fprintf(fp, "_%s(%%rip)", operand_symbol);
            else
                fprintf(fp, "_%s_%d(%%rip)", operand_symbol, operand_symbol_id);
        } else {
            const char *direc = directive[suffix];
            if (operand_symbol_id > 0)
                fprintf(fp, "%s ptr [_%s_%d]", direc, operand_symbol, operand_symbol_id);
            else
                fprintf(fp, "%s ptr [_%s]", direc, operand_symbol);
        }
        break;

    case OPR_LBL:
        if (operand_label_id < 0)
            fprintf(fp, "_%s", operand_label);
        else
            fprintf(fp, "_%s_%d", operand_label, operand_label_id);
        break;

    default:
        break;
    }
}

enum operand regi(enum operand oper, enum operand_size size)
{
    if (is_register(oper) && oper % 5 == 0)
        return oper + size;
    else
        return A_;
}

enum operand arg_reg(int index, int size)
{
    if (index < 0 || index > 5)
        return A_;

    return regi(arg_reg_list[index], size);
}

enum operand imm(long val)
{
    operand_immediate = val;
    return OPR_IMM;
}

enum operand mem(enum operand reg, int offset)
{
    operand_register = reg;
    operand_offset = offset;
    return OPR_MEM;
}

enum operand symb(const char *sym, int id)
{
    operand_symbol = sym;
    operand_symbol_id = id;
    return OPR_SYM;
}

enum operand label(const char *label_name, int id)
{
    operand_label = label_name;
    operand_label_id = id;
    return OPR_LBL;
}

void code1(FILE *fp, enum opecode op)
{
    print_indent(fp);
    print_opecode(fp, op, I64);

    fprintf(fp, "\n");
}

void code2(FILE *fp, enum opecode op, enum operand oper1)
{
    const int size1 = get_operand_size(oper1);
    const int sfx = size1;

    print_indent(fp);
    print_opecode(fp, op, sfx);

    print_separator(fp);
    print_operand(fp, oper1, sfx);

    fprintf(fp, "\n");
}

void code3(FILE *fp, enum opecode op, enum operand oper1, enum operand oper2)
{
    const int size1 = get_operand_size(oper1);
    const int size2 = get_operand_size(oper2);
    const int sfx = size1 > size2 ? size1 : size2;

    print_indent(fp);
    print_opecode(fp, op, sfx);

    print_separator(fp);
    print_operand(fp, oper1, sfx);

    fprintf(fp, ", ");
    print_operand(fp, oper2, sfx);

    fprintf(fp, "\n");
}

/*============================================================================*/

static const char *LABEL_NAME_PREFIX = "LBB";
static const char *STR_LIT_NAME_PREFIX = "L_str";

static enum operand make_label(int block_id, int label_id)
{
    static char buf[64] = {'\0'};
    sprintf(buf, "%s%d", LABEL_NAME_PREFIX, block_id);
    return label(buf, label_id);
}

static void gen_symbol_name(FILE *fp, const char *name, int label_id)
{
    if (label_id < 0)
        fprintf(fp, "_%s", name);
    else
        fprintf(fp, "_%s_%d", name, label_id);
}

static void gen_label_name(FILE *fp, const char *prefix, int block_id, int label_id)
{
    if (block_id < 0 && label_id < 0)
        fprintf(fp, "_%s", prefix);
    else
        fprintf(fp, "_%s%d_%d", prefix, block_id, label_id);
}

static void gen_string_literal_name(FILE *fp, int label_id)
{
    if (label_id < 0)
        fprintf(fp, "_%s", STR_LIT_NAME_PREFIX);
    else
        fprintf(fp, "_%s_%d", STR_LIT_NAME_PREFIX, label_id);
}

static int operand_size(const struct data_type *type)
{
    if (is_char(type))
        return I8;
    if (is_short(type))
        return I16;
    if (is_int(type))
        return I32;
    if (is_long(type))
        return I64;
    if (is_pointer(type))
        return I64;
    if (is_enum(type))
        return I32;
    return I64;
}

static const char *data_name_from_type(const struct data_type *type)
{
    const int size = operand_size(type);
    return data_name[size];
}

static enum operand register_from_type(enum operand oper, const struct data_type *type)
{
    const int size = operand_size(type);
    return regi(oper, size);
}

static int get_mem_offset(const struct ast_node *node)
{
    return node->sym->mem_offset;
}

/* forward declaration */
static void gen_code(FILE *fp, const struct ast_node *node);
static void gen_address(FILE *fp, const struct ast_node *node);
static void gen_load(FILE *fp, const struct ast_node *node,
        enum operand addr, enum operand regist);
static void gen_store_a(FILE *fp, const struct data_type *type,
        enum operand addr, int offset);
static void gen_cast(FILE *fp, const struct ast_node *node);
static void gen_assign_struct(FILE *fp, const struct data_type *type);
static void gen_copy_large_object(FILE *fp, const struct data_type *type,
        enum operand addr, int offset);
static void gen_convert_a(FILE *fp,
        const struct data_type *src, const struct data_type *dst);

static void gen_comment(FILE *fp, const char *cmt)
{
    fprintf(fp, "## %s\n", cmt);
}

static int align_to(int pos, int align)
{
    return ((pos + align - 1) / align) * align;
}

static int is_small_object(const struct data_type *type)
{
    return get_size(type) <= 8;
}

static int is_medium_object(const struct data_type *type)
{
    const int size = get_size(type);
    return size > 8 && size <= 16;
}

static int is_large_object(const struct data_type *type)
{
    return get_size(type) > 16;
}

static void gen_add_stack_pointer(FILE *fp, int byte)
{
    if (!byte)
        return;
    code3(fp, ADD, imm(byte), RSP);
    inc_stack_pointer(byte);
}

static void gen_sub_stack_pointer(FILE *fp, int byte)
{
    if (!byte)
        return;
    code3(fp, SUB, imm(byte), RSP);
    dec_stack_pointer(byte);
}

static void gen_func_param_list_variadic_(FILE *fp)
{
    int i;

    for (i = 0; i < 6; i++) {
        const int disp = -8 * (6 - i);
        const int reg_ = arg_reg(i, I64);
        code3(fp, MOV, reg_, mem(RBP, disp));
    }
}

static int gen_store_param(FILE *fp, const struct symbol *sym, int stored_regs)
{
    const int size = get_size(sym->type);
    const int N8 = size / 8;
    const int N4 = (size - 8 * N8) / 4;
    int offset = 0;
    int r = stored_regs;
    int i;

    code3(fp, MOV, RBP, R10);
    code3(fp, SUB, imm(sym->mem_offset), R10);

    for (i = 0; i < N8; i++) {
        const int reg_ = arg_reg(r, I64);
        code3(fp, MOV, reg_, mem(R10, offset));
        r++;
        offset += 8;
    }

    for (i = 0; i < N4; i++) {
        const int reg_ = arg_reg(r, I32);
        code3(fp, MOV, reg_, mem(R10, offset));
        r++;
        offset += 4;
    }

    return r;
}

static void gen_func_param_list_(FILE *fp, const struct data_type *func_type)
{
    const struct parameter *p;
    int stored_reg_count = 0;
    int stack_offset = 16; /* from rbp */

    int next_fp = 0;

    if (is_large_object(return_type(func_type))) {
        gen_comment(fp, "save address to returning value");
        code2(fp, PUSH, RDI);
        stored_reg_count++;
    }

    for (p = first_param(func_type); p; p = next_param(p)) {
        const struct symbol *sym = p->sym;
        const int param_size = get_size(sym->type);

        const struct data_type *param_type = sym->type;

        if (is_ellipsis(sym))
            break;

        if (is_fpnum(param_type)) {
            const int disp = -1 * sym->mem_offset;
            fprintf(fp, "    movss  %%xmm%d, %d(%%rbp)\n", next_fp, disp);
            next_fp++;
            continue;
        }

        if (param_size <= 8 && stored_reg_count < 6) {
            const int reg_ = arg_reg(stored_reg_count, operand_size(sym->type));
            const int disp = -1 * sym->mem_offset;

            code3(fp, MOV, reg_, mem(RBP, disp));
            stored_reg_count++;
        }
        else if (param_size <= 16 && stored_reg_count < 5) {
            stored_reg_count = gen_store_param(fp, sym, stored_reg_count);
        }
        else {
            /* src from stack */
            code3(fp, MOV, RBP, RAX);
            code3(fp, ADD, imm(stack_offset), RAX);
            gen_copy_large_object(fp, sym->type, RBP, -sym->mem_offset);

            /* 8 byte align */
            stack_offset += align_to(param_size, 8);
        }
    }
}

static void gen_func_param_list(FILE *fp, const struct ast_node *node)
{
    const struct ast_node *func = node->l;

    if (is_variadic(func->sym))
        gen_func_param_list_variadic_(fp);
    else
        gen_func_param_list_(fp, func->type);
}

static int local_area_size = 0;
static void find_max_return_size(const struct ast_node *node, int *max)
{
    if (!node)
        return;

    if (node->kind == NOD_CALL) {
        if (!is_small_object(node->type)) {
            const int size = get_size(node->type);
            if (size > *max)
                *max = size;
        }
    }
    find_max_return_size(node->l, max);
    find_max_return_size(node->r, max);
}

static int get_local_area_size(void)
{
    return local_area_size;
}

static void set_local_area_offset(const struct ast_node *node)
{
    const struct ast_node *func = node->l->l;
    int local_var_size = 0;
    int ret_val_size = 0;

    local_var_size = get_mem_offset(func);

    find_max_return_size(node->r, &ret_val_size);
    /* 16 byte align */
    ret_val_size = align_to(ret_val_size, 16);

    local_area_size = local_var_size + ret_val_size;
}

static void gen_func_prologue(FILE *fp, const struct ast_node *node)
{
    const struct ast_node *func = node->l;

    if (!is_static(func->sym))
        fprintf(fp, "    .global _%s\n", func->sym->name);
    fprintf(fp, "_%s:\n", func->sym->name);
    code2(fp, PUSH, RBP);
    code3(fp, MOV,  RSP, RBP);
    code3(fp, SUB, imm(get_local_area_size()), RSP);
}

static void gen_func_body(FILE *fp, const struct ast_node *node)
{
    gen_code(fp, node);
}

static void gen_func_epilogue(FILE *fp, const struct ast_node *node)
{
    code3(fp, MOV, RBP, RSP);
    code2(fp, POP, RBP);
    code1(fp, RET);
}

struct arg_area {
    const struct ast_node *expr;
    const struct data_type *param_type;
    int offset;
    int size;
    char pass_by_stack;
    char is_fp;
};

static void print_arg_area(const struct arg_area *args, int count)
{
    int i;
    for (i = 0; i < count; i++) {
        const struct arg_area *a = &args[i];
        printf("* %2d | size: %d, offset: %d, pass_by_stack: %d, is_fp: %d\n",
                i, a->size, a->offset, a->pass_by_stack, a->is_fp);
    }
}

static int gen_load_arg(FILE *fp, const struct arg_area *arg, int loaded_regs)
{
    const int size = get_size(arg->expr->type);
    const int N8 = size / 8;
    const int N4 = (size - 8 * N8) / 4;
    int offset = 0;
    int r = loaded_regs;
    int i;

    for (i = 0; i < N8; i++) {
        const int reg_ = arg_reg(r, I64);
        code3(fp, MOV, mem(RSP, arg->offset + offset), reg_);
        r++;
        offset += 8;
    }

    for (i = 0; i < N4; i++) {
        const int reg_ = arg_reg(r, I32);
        code3(fp, MOV, mem(RSP, arg->offset + offset), reg_);
        r++;
        offset += 4;
    }

    return r;
}

static void gen_func_call(FILE *fp, const struct ast_node *node)
{
    /* TODO divide this function */
    const struct symbol *func_sym = node->l->sym;
    const struct data_type *ret_type = node->type;
    struct arg_area *args = NULL;
    int arg_count = node->ival;
    int total_area_size = 0;

    /* use the first register(rdi) for pointer to return value space */
    if (is_large_object(ret_type))
        arg_count++;

    /* allocate arg area */
    args = calloc(arg_count, sizeof(struct arg_area));

    /* compute total arg area size */
    {
        const struct ast_node *list = node->r;
        struct arg_area *arg = &args[arg_count - 1];

        for (; list; list = list->l) {
            const struct ast_node *arg_node = list->r;
            int type_size = 0;

            arg->expr = arg_node->l;
            arg->param_type = arg_node->type;
            type_size = get_size(arg->expr->type);

            if (is_fpnum(arg->expr->type))
                arg->is_fp = 1;

            /* 8 byte align */
            arg->size = align_to(type_size, 8);
            total_area_size += arg->size;

            arg--;
        }
    }

    /* adjust area size */
    {
        const int adjust = !is_stack_aligned();

        if ((total_area_size + 8 * adjust) % 16 != 0)
            total_area_size += 8;
    }

    /* compute offsets */
    {
        /* start from bottom of arg area */
        int reg_offset = total_area_size;
        /* start from top of arg area */
        int mem_offset = 0;
        int used_reg_count = 0;
        int i;

        int used_fp = 0;

        for (i = 0; i < arg_count; i++) {
            struct arg_area *arg = &args[i];

            /* skip for return value */
            if (!arg->expr)
                continue;

            if (arg->is_fp) {
                if (arg->size == 8 && used_fp < 6) {
                    reg_offset -= arg->size;
                    arg->offset = reg_offset;
                    used_fp++;
                }
                continue;
            }

            if (arg->size == 8 && used_reg_count < 6) {
                reg_offset -= arg->size;

                arg->offset = reg_offset;

                used_reg_count++;
            }
            else if (arg->size == 16 && used_reg_count < 5) { /* need 2 regs */
                reg_offset -= arg->size;

                arg->offset = reg_offset;

                used_reg_count += 2;
            }
            else {
                arg->offset = mem_offset;
                arg->pass_by_stack = 1;

                mem_offset += arg->size;
            }
        }
    }

    if (0)
        print_arg_area(args, arg_count);

    {
        int loaded_reg_count = 0;
        int i;

        int loaded_fp = 0;

        gen_comment(fp, "allocate arg area");
        gen_sub_stack_pointer(fp, total_area_size);

        /* eval arg expr */
        gen_comment(fp, "store args");
        for (i = 0; i < arg_count; i++) {
            struct arg_area *arg = &args[i];

            /* skip for return value */
            if (!arg->expr)
                continue;

            gen_code(fp, arg->expr);
            gen_convert_a(fp, arg->expr->type, arg->param_type);
            gen_store_a(fp, arg->param_type, RSP, arg->offset);
        }

        /* load to registers */
        gen_comment(fp, "load args");
        for (i = 0; i < arg_count && loaded_reg_count < 6; i++) {
            struct arg_area *arg = &args[i];

            if (!arg->expr) {
                /* large return value */
                const int offset = -get_local_area_size();
                const int reg_ = arg_reg(0, I64);
                gen_comment(fp, "load address to returned value");
                code3(fp, LEA, mem(RBP, offset), reg_);
                loaded_reg_count++;
                continue;
            }

            if (arg->pass_by_stack)
                continue;

            if (arg->is_fp) {
                if (is_float(arg->param_type))
                    fprintf(fp, "    movss  %d(%%rsp), %%xmm%d\n", arg->offset, loaded_fp);
                else
                    fprintf(fp, "    movsd  %d(%%rsp), %%xmm%d\n", arg->offset, loaded_fp);
                loaded_fp++;
                continue;
            }

            if (arg->size > 8) {
                loaded_reg_count = gen_load_arg(fp, arg, loaded_reg_count);
            } else {
                const int reg_ = arg_reg(loaded_reg_count, operand_size(arg->param_type));
                code3(fp, MOV, mem(RSP, arg->offset), reg_);
                loaded_reg_count++;
            }
        }

        /* number of fp */
        if (is_variadic(func_sym))
            code3(fp, MOV, imm(loaded_fp), AL);

        /* call */
        gen_comment(fp, "call");
        if (is_pointer(func_sym->type)) {
            const int id = is_static(func_sym) ? func_sym->id : -1;
            if (is_global_var(func_sym))
                code2(fp, CALL, symb(func_sym->name, id));
            else
                code2(fp, CALL, mem(RBP, -func_sym->mem_offset));
        } else {
            code2(fp, CALL, label(func_sym->name, -1));
        }

        gen_comment(fp, "free up arg area");
        gen_add_stack_pointer(fp, total_area_size);
    }

    free(args);

    if (is_medium_object(ret_type)) {
        const int offset = -get_local_area_size();

        gen_comment(fp, "store returned value");
        code3(fp, MOV, RAX, mem(RBP, offset));
        code3(fp, MOV, RDX, mem(RBP, offset + 8));
        gen_comment(fp, "load address to returned value");
        code3(fp, LEA, mem(RBP, offset), RAX);
    }
}

static void gen_builtin_va_start(FILE *fp)
{
    fprintf(fp, "## __builtin_va_start\n");
    /* pop arguments */
    fprintf(fp, "    pop    %%rax ## &va_list\n");
    fprintf(fp, "    pop    %%rdx ## &last\n");
    /* gp_offset */
    fprintf(fp, "    leaq   -48(%%rbp), %%rdi ## gp_offset\n");
    fprintf(fp, "    subq   %%rdi, %%rdx      ## gp_offset\n");
    fprintf(fp, "    addq   $8, %%rdx         ## gp_offset\n");
    fprintf(fp, "    movq   %%rdx, (%%rax)    ## gp_offset\n");
    /* fp_offset */
    fprintf(fp, "    movl   $48, 4(%%rax)     ## fp_offset\n");
    /* overflow_arg_area */
    fprintf(fp, "    leaq   16(%%rbp), %%rdi  ## overflow_arg_area\n");
    fprintf(fp, "    movq   %%rdi, 8(%%rax)   ## overflow_arg_area\n");
    /* reg_save_area */
    fprintf(fp, "    leaq   -48(%%rbp), %%rdi ## reg_save_area\n");
    fprintf(fp, "    movq   %%rdi, 16(%%rax)  ## reg_save_area\n");
    fprintf(fp, "## end of __builtin_va_start\n");
}

static void gen_func_call_builtin(FILE *fp, const struct ast_node *node)
{
    if (!node)
        return;

    switch (node->kind) {

    case NOD_CALL:
        {
            const struct symbol *func_sym = node->l->sym;

            /* push args to stack */
            gen_func_call_builtin(fp, node->r);

            /* call */
            if (!strcmp(func_sym->name, "__builtin_va_start"))
                gen_builtin_va_start(fp);

            return;
        }

    case NOD_ARG:
        /* push args */
        gen_code(fp, node->l);
        /* no count pushes and pops as builtins are not function calls */
        code3(fp, SUB, imm(8), RSP);
        code3(fp, MOV, RAX, mem(RSP, 0));
        return;

    default:
        /* walk tree from the rightmost arg */
        gen_func_call_builtin(fp, node->r);
        gen_func_call_builtin(fp, node->l);
        return;
    }
}

static void gen_label(FILE *fp, int block_id, int label_id)
{
    gen_label_name(fp, LABEL_NAME_PREFIX, block_id, label_id);
    fprintf(fp, ":\n");
}

static void gen_ident(FILE *fp, const struct ast_node *node)
{
    const struct symbol *sym;

    if (!node || !node->sym)
        return;

    sym = node->sym;

    if (is_global_var(sym)) {
        const int id = is_static(sym) ? sym->id : -1;
        if (is_array(node->type)) {
            code3(fp, LEA, symb(sym->name, id), RAX);
        } else {
            /* TODO come up with better idea */
            if (!strcmp(sym->name, "__stdinp") ||
                !strcmp(sym->name, "__stdoutp") ||
                !strcmp(sym->name, "__stderrp")) {
                const int a_ = regi(A_, I64);
                char buf[128] = {'\0'};
                sprintf(buf, "%s@GOTPCREL", sym->name);
                code3(fp, MOV, symb(buf, -1), a_);
                code3(fp, MOV, mem(RAX, 0), RAX);
            } else {
                const int a_ = register_from_type(A_, node->type);
                code3(fp, MOV, symb(sym->name, id), a_);
            }
        }
    }
    else if (is_enumerator(sym)) {
        code3(fp, MOV, imm(get_mem_offset(node)), EAX);
    }
    else if (is_func(sym)) {
        code3(fp, LEA, symb(sym->name, -1), RAX);
    }
    else {
        const int disp = -get_mem_offset(node);

        if (is_array(node->type))
            code3(fp, LEA, mem(RBP, disp), RAX);
        else if (is_fpnum(node->type)) {
            fprintf(fp, "    movss  %d(%%rbp), %%xmm0\n", disp);
        }
        else
            gen_load(fp, node, mem(RBP, disp), A_);
    }
}

static void gen_ident_lvalue(FILE *fp, const struct ast_node *node)
{
    const struct symbol *sym;

    if (!node || !node->sym)
        return;

    sym = node->sym;

    if (is_global_var(sym)) {
        const int id = is_static(sym) ? sym->id : -1;
        code3(fp, LEA, symb(sym->name, id), RAX);
    } else {
        code3(fp, MOV, RBP, RAX);
        code3(fp, SUB, imm(get_mem_offset(node)), RAX);
    }
}

static void gen_address(FILE *fp, const struct ast_node *node)
{
    if (node == NULL)
        return;

    switch (node->kind) {

        /* TODO need this for initialization. may not need this for IR */
    case NOD_DECL_IDENT:
    case NOD_IDENT:
        gen_ident_lvalue(fp, node);
        break;

    case NOD_DEREF:
        gen_code(fp, node->l);
        break;

    case NOD_STRUCT_REF:
        {
            const int disp = get_mem_offset(node->r);
            gen_address(fp, node->l);
            code3(fp, ADD, imm(disp), RAX);
        }
        break;

    default:
        gen_comment(fp, "not an lvalue");
        break;
    }
}

static void gen_load(FILE *fp, const struct ast_node *node,
        enum operand addr, enum operand regist)
{
    const int reg_ = register_from_type(regist, node->type);

    /* array objects cannot be loaded in registers, and converted to pointers */
    if (is_array(node->type))
        return;
    /* large struct objects cannot be loaded in registers,
     * and the compiler handle it via pointer */
    if (!is_small_object(node->type))
        return;

    gen_comment(fp, "load");
    code3(fp, MOV, addr, reg_);

    if (is_char(node->type) || is_short(node->type)) {
        gen_comment(fp, "cast");
        gen_cast(fp, node);
    }
}

static void gen_store_a(FILE *fp, const struct data_type *type,
        enum operand addr, int offset)
{
    const int a_ = register_from_type(A_, type);

    if (is_small_object(type)) {
        if (is_float(type))
        {
            if (addr == RDX)
                fprintf(fp, "    movss  %%xmm0, %d(%%rdx)\n", offset);
            if (addr == RSP)
                fprintf(fp, "    movss  %%xmm0, %d(%%rsp)\n", offset);
        }
        else
        {
            code3(fp, MOV, a_, mem(addr, offset));
        }
    }
    else {
        gen_copy_large_object(fp, type, addr, offset);
    }
}

static void gen_div(FILE *fp, const struct ast_node *node, enum operand divider)
{
    const int d_ = register_from_type(D_, node->type);
    const int divider_ = register_from_type(divider, node->type);

    /* rax -> rdx:rax (zero extend) */
    if (is_unsigned(node->type)) {
        code3(fp, XOR, d_, d_);
        code2(fp, DIV, divider_);
        return;
    }

    /* rax -> rdx:rax (signed extend) */
    if (is_long(node->type))
        code1(fp, CQTO);
    else
        code1(fp, CLTD);

    code2(fp, IDIV, divider_);
}

static void gen_preincdec(FILE *fp, const struct ast_node *node, enum opecode op)
{
    const int a_ = register_from_type(A_, node->type);
    const int d_ = register_from_type(D_, node->type);

    int stride = 1;
    if (is_pointer(node->type))
        stride = get_size(underlying(node->type));

    gen_address(fp, node->l);
    code3(fp, MOV, mem(RAX, 0), d_);
    code3(fp, op, imm(stride), d_);
    code3(fp, MOV, d_, mem(RAX, 0));
    code3(fp, MOV, d_, a_);
}

static void gen_postincdec(FILE *fp, const struct ast_node *node, enum opecode op)
{
    const int a_ = register_from_type(A_, node->type);
    const int c_ = register_from_type(C_, node->type);

    int stride = 1;
    if (is_pointer(node->type))
        stride = get_size(underlying(node->type));

    gen_address(fp, node->l);
    code3(fp, MOV, RAX, RDX);
    code3(fp, MOV, mem(RAX, 0), a_);
    code3(fp, MOV, a_, c_);
    code3(fp, op, imm(stride), c_);
    code3(fp, MOV, c_, mem(RDX, 0));
}

static void gen_relational(FILE *fp, const struct ast_node *node, enum opecode op)
{
    const int a_ = register_from_type(A_, node->type);
    const int d_ = register_from_type(D_, node->type);

    gen_code(fp, node->l);
    code2(fp, PUSH, RAX);
    gen_code(fp, node->r);
    code3(fp, MOV, a_, d_);
    code2(fp, POP, RAX);
    code3(fp, CMP, d_, a_);
    code2(fp, op, AL);
    code3(fp, MOVZB, AL, a_);
}

static void gen_equality(FILE *fp, const struct ast_node *node, enum opecode op)
{
    const int a_ = register_from_type(A_, node->type);
    const int d_ = register_from_type(D_, node->type);

    gen_code(fp, node->l);
    code2(fp, PUSH, RAX);
    gen_code(fp, node->r);
    code2(fp, POP, RDX);
    code3(fp, CMP, d_, a_);
    code2(fp, op,   AL);
    code3(fp, MOVZB, AL, a_);
}

enum jump_kind {
    JMP_RETURN = 0,
    JMP_ENTER,
    JMP_EXIT,
    JMP_ELSE,
    JMP_CONTINUE,
    JMP_OFFSET = 100
};

struct jump_scope {
    int curr;
    int brk;
    int conti;
    int func;
};

static int jump_id(const struct ast_node *node)
{
    return JMP_OFFSET + node->sym->id;
}

static void gen_switch_table_(FILE *fp, const struct ast_node *node,
        int switch_scope, const struct data_type *ctrl_type)
{
    if (!node)
        return;

    switch (node->kind) {

    case NOD_SWITCH:
        /* skip nested switch */
        return;

    case NOD_CASE:
        {
            const int a_ = register_from_type(A_, node->type);

            code3(fp, CMP, imm(node->l->ival), a_);
            code2(fp, JE,  make_label(switch_scope, jump_id(node)));
            /* check next statement if it is another case statement */
            gen_switch_table_(fp, node->r, switch_scope, ctrl_type);
        }
        return;

    case NOD_DEFAULT:
        code2(fp, JMP, make_label(switch_scope, jump_id(node)));
        return;

    case NOD_COMPOUND:
    case NOD_LIST:
        gen_switch_table_(fp, node->l, switch_scope, ctrl_type);
        gen_switch_table_(fp, node->r, switch_scope, ctrl_type);
        break;

    default:
        break;
    }
}

static void gen_switch_table(FILE *fp, const struct ast_node *node, int switch_scope)
{
    gen_comment(fp, "begin jump table");
    gen_switch_table_(fp, node->r, switch_scope, node->l->type);
    /* for switch without default */
    code2(fp, JMP, make_label(switch_scope, JMP_EXIT));
    gen_comment(fp, "end jump table");
}

static void gen_cast(FILE *fp, const struct ast_node *node)
{
    struct data_type *type = node->type;

    if (is_pointer(type))
        return;

    if (is_char(type)) {
        if (is_unsigned(type))
            code3(fp, MOVZB, AL, RAX);
        else
            code3(fp, MOVSB, AL, RAX);
    }
    else if (is_short(type)) {
        if (is_unsigned(type))
            code3(fp, MOVZW, AX, RAX);
        else
            code3(fp, MOVSW, AX, RAX);
    }
    else if (is_int(type)) {
        if (is_unsigned(type))
            code3(fp, MOV,   EAX, EAX);
        else
            code3(fp, MOVSL, EAX, RAX);
    }
}

static void gen_convert_a(FILE *fp, const struct data_type *src, const struct data_type *dst)
{
    if (is_identical(src, dst))
        return;

    if (is_char(src)) {
        const int SRC_ = register_from_type(A_, src);
        const int DST_ = register_from_type(A_, dst);
        const int MOV_ = is_unsigned(src) ? MOVZB : MOVSB;
        code3(fp, MOV_, SRC_, DST_);
    }
    else if (is_short(src)) {
        if (is_long(dst) || is_int(dst)) {
            if (is_unsigned(src))
                code3(fp, MOVZW, AX, RAX);
            else
                code3(fp, MOVSW, AX, RAX);
        }
    }
    else if (is_int(src)) {
        if (is_long(dst) || is_int(dst)) {
            if (is_unsigned(src))
                code3(fp, MOV,   EAX, EAX);
            else
                code3(fp, MOVSL, EAX, RAX);
        }
        else if (is_float(dst)) {
            fprintf(fp, "    cvtsi2ssl  %%eax, %%xmm0\n");
        }
    }
    else if (is_float(src)) {
        if (is_void(dst)) {
            fprintf(fp, "    cvtss2sd  %%xmm0, %%xmm0\n");
        }
    }
}

static void gen_assign_struct(FILE *fp, const struct data_type *type)
{
    /* assuming src addess is in rax, and dest address is in rdx */
    const int size = get_size(type);
    const int N8 = size / 8;
    const int N4 = (size - 8 * N8) / 4;
    int offset = 0;
    int i;

    for (i = 0; i < N8; i++) {
        code3(fp, MOV, mem(RAX, offset), RDI);
        code3(fp, MOV, RDI, mem(RDX, offset));
        offset += 8;
    }
    for (i = 0; i < N4; i++) {
        code3(fp, MOV, mem(RAX, offset), EDI);
        code3(fp, MOV, EDI, mem(RDX, offset));
        offset += 4;
    }
}

static void gen_copy_large_object(FILE *fp, const struct data_type *type,
        enum operand addr, int offset)
{
    /* assuming src addess is in rax */
    const int size = get_size(type);
    const int N8 = size / 8;
    const int N4 = (size - 8 * N8) / 4;
    int disp = 0;
    int i;

    for (i = 0; i < N8; i++) {
        code3(fp, MOV, mem(RAX, disp), R10);
        code3(fp, MOV, R10, mem(addr, offset + disp));
        disp += 8;
    }
    for (i = 0; i < N4; i++) {
        code3(fp, MOV, mem(RAX, disp), R10D);
        code3(fp, MOV, R10D, mem(addr, offset + disp));
        disp += 4;
    }
}

static void gen_init_scalar_local(FILE *fp, const struct data_type *type,
        const struct ast_node *ident, int offset, const struct ast_node *expr)
{
    gen_comment(fp, "local scalar init");

    /* ident */
    gen_address(fp, ident);
    if (offset > 0)
        code3(fp, ADD, imm(offset), RAX);
    code2(fp, PUSH, RAX);

    if (expr) {
        const int a_ = register_from_type(A_, type);

        /* init expr */
        gen_code(fp, expr);
        /* assign expr */
        code2(fp, POP,  RDX);
        gen_comment(fp, "assign object");
        if (is_small_object(type))
            code3(fp, MOV, a_, mem(RDX, 0));
        else
            gen_assign_struct(fp, type);
    } else {
        const int d_ = register_from_type(D_, type);
        /* assign zero */
        /* need pop to align */
        code2(fp, POP,  RAX);
        code3(fp, MOV, imm(0), d_);
        code3(fp, MOV, d_, mem(RAX, 0));
    }
}

struct memory_bit;
struct memory_bit {
    const struct ast_node *init;
    struct memory_bit *next;
    int width;
    int offset;
};

struct memory_byte {
    const struct ast_node *init;
    const struct data_type *type;
    int is_written;
    int written_size;
    struct memory_bit *bit;
};

struct object_byte {
    struct memory_byte *bytes;
    struct symbol *sym;
    int size;
};

static void append_new_bit(struct memory_byte *byte,
        int bit_width, int bit_offset)
{
    struct memory_bit *bit;
    struct memory_bit *new_bit = calloc(1, sizeof(struct memory_bit));
    new_bit->width = bit_width;
    new_bit->offset = bit_offset;

    if (!byte->bit) {
        byte->bit = new_bit;
        return;
    }

    for (bit = byte->bit; bit->next; bit = bit->next)
        ;
    bit->next = new_bit;
}

static void free_memory_bit(struct memory_bit *bit)
{
    struct memory_bit *b = bit, *tmp;

    while (b) {
        tmp = b->next;
        free(b);
        b = tmp;
    }
}

static void gen_init_bit_local(FILE *fp,
        const struct ast_node *ident, int offset, const struct memory_bit *mem_bit)
{
    const struct memory_bit *bit;

    gen_comment(fp, "local bit init");

    /* ident */
    gen_address(fp, ident);
    if (offset > 0)
        code3(fp, ADD, imm(offset), RAX);
    /* zero clear 4 bytes */
    code3(fp, MOV, imm(0), ECX);
    code3(fp, MOV, ECX, mem(RAX, 0));

    for (bit = mem_bit; bit; bit = bit->next) {
        const int bit_width = bit->width;
        const int bit_offset = bit->offset;
        int mask;
        mask = ~1 << (bit_width - 1);
        mask = ~mask;
        mask <<= bit_offset;
        mask = ~mask;

        /* ident */
        gen_address(fp, ident);
        if (offset > 0)
            code3(fp, ADD, imm(offset), RAX);
        /* push dest addr */
        code2(fp, PUSH, RAX);
        /* init expr */
        gen_code(fp, bit->init);
        /* assign expr */
        code2(fp, POP,  RDX);

        gen_comment(fp, "assign bit field");
        code3(fp, MOV, EAX, ECX);
        code3(fp, MOV, ECX, R10D);
        code3(fp, SHL, imm(bit_offset), ECX);
        code3(fp, MOV, mem(RDX, 0), EAX);
        code3(fp, AND, imm(mask), EAX);
        code3(fp, OR,  ECX, EAX);
        code3(fp, MOV, EAX, mem(RDX, 0));
        code3(fp, MOV, R10D, EAX);
    }
}

static void print_object(struct object_byte *obj)
{
    char type_name[128] = {'\0'};
    int i;

    make_type_name(obj->sym->type, type_name);
    printf("%s %s:\n", type_name, obj->sym->name);
    for (i = 0; i < obj->size; i++) {
        const struct memory_byte *byte = &obj->bytes[i];
        printf("    [%04d] is_written: %d written_size: %d init: %p\n",
                i, byte->is_written, byte->written_size, (void *) byte->init);

        if (byte->bit) {
            struct memory_bit *bit = byte->bit;
            for (; bit; bit = bit->next) {
                printf("           ");
                printf("[%02d] bit_width: %d ---------- init: %p\n",
                        bit->offset, bit->width, (void *) bit->init);
            }
        }
    }
}

static void zero_clear_bytes(struct memory_byte *bytes, const struct data_type *type)
{
    if (is_array(type)) {
        const int len = get_array_length(type);
        const int elem_size = get_size(underlying(type));
        int i;

        for (i = 0; i < len; i++) {
            const int base = elem_size * i;

            zero_clear_bytes(bytes + base, underlying(type));
        }
    }
    else if (is_struct(type)) {
        const struct member *m;
        for (m = first_member(type); m; m = next_member(m)) {
            const struct symbol *sym = m->sym;

            if (is_bitfield(sym))
                append_new_bit(&bytes[sym->mem_offset],
                        sym->bit_width, sym->bit_offset);
            zero_clear_bytes(bytes + sym->mem_offset, sym->type);
        }
    }
    else if (is_union(type)) {
        const struct member *m = first_member(type);
        zero_clear_bytes(bytes, m->sym->type);
    }
    else {
        /* scalar */
        const int size = get_size(type);
        int i;
        for (i = 0; i < size; i++) {
            bytes[i].is_written = 1;
            bytes[i].written_size = (i == 0) ? size : 0;
            bytes[i].type = type;
        }
    }
}

static void init_object_byte(struct object_byte *obj, const struct ast_node *ident)
{
    const int size = get_size(ident->type);

    obj->bytes = calloc(size, sizeof(struct memory_byte));
    obj->size = size;
    obj->sym = ident->sym;

    zero_clear_bytes(obj->bytes, obj->sym->type);
}

static void free_object_byte(struct object_byte *obj)
{
    struct object_byte o = {0};
    int i;
    for (i = 0; i < obj->size; i++) {
        if (obj->bytes[i].bit) {
            free_memory_bit(obj->bytes[i].bit);
            /* TODO improve this un-const cast */
            free_ast_node((struct ast_node *) obj->bytes[i].init);
        }
    }
    free(obj->bytes);
    *obj = o;
}

static void assign_init(struct memory_byte *base,
        const struct data_type *type, const struct ast_node *expr)
{
    if (!expr)
        return;

    switch (expr->kind) {

    case NOD_INIT:
        {
            /* move cursor by designator */
            const int offset = expr->l->ival;

            assign_init(base + offset, underlying(type), expr->r);
        }
        break;

    case NOD_LIST:
        /* pass to the next initializer */
        assign_init(base, type, expr->l);
        assign_init(base, type, expr->r);
        break;

    default:
        if (is_struct_or_union(expr->type)) {
            /* init by struct object */
            const int size = get_size(expr->type);
            int i;

            base->init = expr;

            for (i = 0; i < size; i++) {
                /* override byte properties as if struct object is a scaler value */
                base[i].is_written = 1;
                base[i].written_size = (i == 0) ? size : 0;
                base[i].type = expr->type;
            }
        }
        else {
            const int size = base->written_size;
            int i;

            if (base->bit) {
                /* assign initializer to bit */
                struct memory_bit *bit;
                for (bit = base->bit; bit; bit = bit->next)
                    if (!bit->init)
                        break;
                bit->init = expr;
            }
            else {
                /* assign initializer to byte */
                base->init = expr;
            }

            for (i = 0; i < size; i++)
                base[i].is_written = 1;
        }
        break;
    }
}

static long eval_const_expr__(const struct ast_node *node)
{
    long l, r;

    if (!node)
        return 0;

    switch (node->kind) {

    case NOD_ADD:
        l = eval_const_expr__(node->l);
        r = eval_const_expr__(node->r);
        return l + r;

    case NOD_SUB:
        l = eval_const_expr__(node->l);
        r = eval_const_expr__(node->r);
        return l - r;

    case NOD_MUL:
        l = eval_const_expr__(node->l);
        r = eval_const_expr__(node->r);
        return l * r;

    case NOD_DIV:
        l = eval_const_expr__(node->l);
        r = eval_const_expr__(node->r);
        return l / r;

    case NOD_MOD:
        l = eval_const_expr__(node->l);
        r = eval_const_expr__(node->r);
        return l % r;

    case NOD_SHL:
        l = eval_const_expr__(node->l);
        r = eval_const_expr__(node->r);
        return l << r;

    case NOD_SHR:
        l = eval_const_expr__(node->l);
        r = eval_const_expr__(node->r);
        return l >> r;

    case NOD_SIZEOF:
    case NOD_NUM:
        return node->ival;

    default:
        return 0;
    }
}

static void gen_init_scalar_global(FILE *fp, const struct data_type *type,
        const struct ast_node *expr)
{
    const char *szname = data_name_from_type(type);

    if (!expr) {
        fprintf(fp, "    .%s 0\n", szname);
        return;
    }

    switch (expr->kind) {

    case NOD_NUM:
        fprintf(fp, "    .%s %ld\n", szname, expr->ival);
        break;

    case NOD_IDENT:
        if (expr->sym && is_enumerator(expr->sym))
            fprintf(fp, "    .%s %d\n", szname, get_mem_offset(expr));
        /* initialize with const global variable */
        if (expr->sym && is_global_var(expr->sym)) {
            /* TODO make function taking sym */
            const struct symbol *sym = expr->sym;
            fprintf(fp, "    .%s ", szname);
            if (is_static(sym))
                gen_symbol_name(fp, sym->name, sym->id);
            else
                gen_symbol_name(fp, sym->name, -1);
            fprintf(fp, "\n");
        }
        /* initialize with function name */
        if (expr->sym && is_func(expr->sym)) {
            fprintf(fp, "    .%s ", szname);
            gen_symbol_name(fp, expr->sym->name, -1);
            fprintf(fp, "\n");
        }
        break;

    case NOD_ADDR:
        {
            /* TODO make function taking sym */
            const struct symbol *sym = expr->l->sym;
            fprintf(fp, "    .%s ", szname);
            if (is_static(sym))
                gen_symbol_name(fp, sym->name, sym->id);
            else
                gen_symbol_name(fp, sym->name, -1);
            fprintf(fp, "\n");
        }
        break;

    case NOD_CAST:
        /* TODO come up better way to handle scalar universaly */
        gen_init_scalar_global(fp, type, expr->r);
        break;

    case NOD_STRING:
        fprintf(fp, "    .%s ", szname);
        gen_string_literal_name(fp, expr->sym->id);
        fprintf(fp, "\n");
        break;

    case NOD_ADD:
    case NOD_SUB:
    case NOD_MUL:
    case NOD_DIV:
    case NOD_MOD:
        /* TODO isolate eval_const_expr() */
        fprintf(fp, "    .%s %ld\n", szname, eval_const_expr__(expr));
        break;

    default:
        /* TODO put an assertion */
        break;
    }
}

static void gen_object_byte(FILE *fp, const struct object_byte *obj)
{
    int i;

    {
        struct symbol *sym = obj->sym;
        int id = sym->id;

        /* TODO make function taking sym */
        if (!is_static(sym)) {
            id = -1;
            fprintf(fp, "    .global ");
            gen_symbol_name(fp, sym->name, id);
            fprintf(fp, "\n");
        }
        gen_symbol_name(fp, sym->name, id);
        fprintf(fp, ":\n");
    }

    for (i = 0; i < obj->size; i++) {
        const struct memory_byte *byte = &obj->bytes[i];

        if (!byte->is_written) {
            /* TODO count skipping bytes */
            fprintf(fp, "    .zero 1\n");
            continue;
        }

        if (byte->written_size > 0)
            gen_init_scalar_global(fp, byte->type, byte->init);
    }

    fprintf(fp, "\n");
}

static void gen_initializer(FILE *fp,
        const struct ast_node *ident, const struct ast_node *init)
{
    struct object_byte obj = {0};

    init_object_byte(&obj, ident);
    assign_init(obj.bytes, ident->type, init);

    if (0)
        print_object(&obj);

    if (is_global_var(ident->sym) && !is_extern(ident->sym)) {
        int i;

        /* combine all bit fields within 4 bytes and generate an integer */
        for (i = 0; i < obj.size; i++) {
            struct memory_byte *byte = &obj.bytes[i];

            if (byte->bit) {
                const struct memory_bit *bit = byte->bit;
                struct ast_node *n;
                int byte_data = 0;

                for (; bit; bit = bit->next) {
                    int val = eval_const_expr__(bit->init);
                    int mask;
                    mask = ~1 << bit->width;
                    mask = ~mask;

                    val &= mask;
                    val <<= bit->offset;
                    byte_data |= val;
                }
                /* TODO improve this node will be strayed */
                n = new_ast_node(NOD_NUM, NULL, NULL);
                n->ival = byte_data;
                byte->init = n;
            }
        }

        gen_object_byte(fp, &obj);
    }
    else if (is_local_var(ident->sym) && init) {
        int i;

        for (i = 0; i < obj.size; i++) {
            const struct memory_byte *byte = &obj.bytes[i];

            if (byte->written_size > 0) {
                if (byte->bit)
                    gen_init_bit_local(fp, ident, i, byte->bit);
                else
                    gen_init_scalar_local(fp, byte->type, ident, i, byte->init);
            }
        }
    }

    free_object_byte(&obj);
}

static void gen_code(FILE *fp, const struct ast_node *node)
{
    static struct jump_scope scope = {0};
    struct jump_scope tmp = {0};
    static int next_scope = 0;

    int a_, c_, d_, di_;

    if (node == NULL)
        return;

    a_  = register_from_type(A_, node->type);
    c_  = register_from_type(C_, node->type);
    d_  = register_from_type(D_, node->type);
    di_ = register_from_type(DI_, node->type);

    switch (node->kind) {

    case NOD_LIST:
    case NOD_COMPOUND:
        gen_code(fp, node->l);
        gen_code(fp, node->r);
        break;

    case NOD_FOR:
        tmp = scope;
        scope.curr = next_scope++;
        scope.brk = scope.curr;
        scope.conti = scope.curr;

        gen_code(fp, node->l);
        gen_code(fp, node->r);

        scope = tmp;
        break;

    case NOD_FOR_PRE_COND:
        /* pre */
        gen_comment(fp, "for-pre");
        gen_code(fp, node->l);
        /* cond */
        gen_comment(fp, "for-cond");
        gen_label(fp, scope.curr, JMP_ENTER);
        gen_code(fp, node->r);
        code3(fp, CMP, imm(0), a_);
        code2(fp, JE, make_label(scope.curr, JMP_EXIT));
        break;

    case NOD_FOR_BODY_POST:
        /* body */
        gen_comment(fp, "for-body");
        gen_code(fp, node->l);
        /* post */
        gen_comment(fp, "for-post");
        gen_label(fp, scope.curr, JMP_CONTINUE);
        gen_code(fp, node->r);
        code2(fp, JMP, make_label(scope.curr, JMP_ENTER));
        gen_label(fp, scope.curr, JMP_EXIT);
        break;

    case NOD_WHILE:
        tmp = scope;
        scope.curr = next_scope++;
        scope.brk = scope.curr;
        scope.conti = scope.curr;

        gen_comment(fp, "while-cond");
        gen_label(fp, scope.curr, JMP_CONTINUE);
        gen_code(fp, node->l);
        code3(fp, CMP, imm(0), a_);
        code2(fp, JE,  make_label(scope.curr, JMP_EXIT));
        gen_comment(fp, "while-body");
        gen_code(fp, node->r);
        code2(fp, JMP, make_label(scope.curr, JMP_CONTINUE));
        gen_label(fp, scope.curr, JMP_EXIT);

        scope = tmp;
        break;

    case NOD_DOWHILE:
        tmp = scope;
        scope.curr = next_scope++;
        scope.brk = scope.curr;
        scope.conti = scope.curr;

        gen_comment(fp, "do-while-body");
        gen_label(fp, scope.curr, JMP_ENTER);
        gen_code(fp, node->l);
        gen_comment(fp, "do-while-cond");
        gen_label(fp, scope.curr, JMP_CONTINUE);
        gen_code(fp, node->r);
        code3(fp, CMP, imm(0), a_);
        code2(fp, JE,  make_label(scope.curr, JMP_EXIT));
        code2(fp, JMP, make_label(scope.curr, JMP_ENTER));
        gen_label(fp, scope.curr, JMP_EXIT);

        scope = tmp;
        break;

    case NOD_IF:
        /* if */
        tmp = scope;
        scope.curr = next_scope++;

        gen_comment(fp, "if-cond");
        gen_code(fp, node->l);
        code3(fp, CMP, imm(0), a_);
        code2(fp, JE,  make_label(scope.curr, JMP_ELSE));
        gen_code(fp, node->r);

        scope = tmp;
        break;

    case NOD_IF_THEN:
        /* then */
        gen_comment(fp, "if-then");
        gen_code(fp, node->l);
        code2(fp, JMP, make_label(scope.curr, JMP_EXIT));
        /* else */
        gen_comment(fp, "if-else");
        gen_label(fp, scope.curr, JMP_ELSE);
        gen_code(fp, node->r);
        gen_label(fp, scope.curr, JMP_EXIT);
        break;

    case NOD_SWITCH:
        tmp = scope;
        scope.curr = next_scope++;
        scope.brk = scope.curr;

        gen_comment(fp, "switch-value");
        gen_code(fp, node->l);
        gen_switch_table(fp, node, scope.curr);
        gen_code(fp, node->r);
        gen_label(fp, scope.curr, JMP_EXIT);

        scope = tmp;
        break;

    case NOD_CASE:
    case NOD_DEFAULT:
        gen_label(fp, scope.curr, jump_id(node));
        gen_code(fp, node->r);
        break;

    case NOD_RETURN:
        gen_code(fp, node->l);

        if (is_medium_object(node->type)) {
            /* use rax and rdx */
            gen_comment(fp, "load returning value");
            code3(fp, MOV, RAX, RSI);
            code3(fp, MOV, mem(RSI, 0), RAX);
            code3(fp, MOV, mem(RSI, 8), RDX);
        }
        else if (is_large_object(node->type)) {
            gen_comment(fp, "fill returning value");
            code2(fp, POP, RDX);
            gen_assign_struct(fp, node->type);
            gen_comment(fp, "load address to returning value");
            code3(fp, MOV, RDX, RAX);
        }

        code2(fp, JMP, make_label(scope.func, JMP_RETURN));
        break;

    case NOD_BREAK:
        code2(fp, JMP, make_label(scope.brk, JMP_EXIT));
        break;

    case NOD_CONTINUE:
        code2(fp, JMP, make_label(scope.conti, JMP_CONTINUE));
        break;

    case NOD_LABEL:
        gen_label(fp, scope.func, jump_id(node->l));
        gen_code(fp, node->r);
        break;

    case NOD_GOTO:
        code2(fp, JMP, make_label(scope.func, jump_id(node->l)));
        break;

    case NOD_IDENT:
        if (is_small_object(node->type))
            gen_ident(fp, node);
        else
            gen_address(fp, node);
        break;

    case NOD_DECL_IDENT:
        if (is_local_var(node->sym) && node->l)
            gen_initializer(fp, node, node->l);
        break;

    case NOD_STRUCT_REF:
        gen_address(fp, node);
        gen_load(fp, node, mem(RAX, 0), A_);

        if (is_bitfield(node->r->sym)) {
            const struct symbol *sym = node->r->sym;
            const int sl = 32 - sym->bit_width - sym->bit_offset;
            const int sr = 32 - sym->bit_width;
            code3(fp, SHL, imm(sl), EAX);
            if (is_unsigned(node->type))
                code3(fp, SHR, imm(sr), EAX);
            else
                code3(fp, SAR, imm(sr), EAX);
        }

        break;

    case NOD_CALL:
        if (is_builtin(node->l->sym))
            gen_func_call_builtin(fp, node);
        else
            gen_func_call(fp, node);
        break;

    case NOD_FUNC_DEF:
        tmp = scope;
        scope.curr = next_scope++;
        scope.func = scope.curr;

        set_local_area_offset(node);

        gen_func_prologue(fp, node->l);
        gen_comment(fp, "func params");
        gen_func_param_list(fp, node->l);
        gen_comment(fp, "func body");
        gen_func_body(fp, node->r);
        gen_label(fp, scope.curr, JMP_RETURN);
        gen_func_epilogue(fp, node->r);

        scope = tmp;
        break;

    case NOD_ASSIGN:
        gen_comment(fp, "assign");
        gen_address(fp, node->l);
        code2(fp, PUSH, RAX);
        gen_code(fp, node->r);
        code2(fp, POP,  RDX);

        if (node->l->kind == NOD_STRUCT_REF && is_bitfield(node->l->r->sym)) {
            const struct symbol *sym = node->l->r->sym;
            int mask;
            mask = ~1 << (sym->bit_width - 1);
            mask = ~mask;
            mask <<= sym->bit_offset;
            mask = ~mask;

            gen_comment(fp, "bit field");
            code3(fp, MOV, EAX, ECX);
            code3(fp, MOV, ECX, R10D);
            code3(fp, SHL, imm(sym->bit_offset), ECX);
            code3(fp, MOV, mem(RDX, 0), EAX);
            code3(fp, AND, imm(mask), EAX);
            code3(fp, OR,  ECX, EAX);
            code3(fp, MOV, EAX, mem(RDX, 0));
            code3(fp, MOV, R10D, EAX);
            break;
        }

        gen_convert_a(fp, node->r->type, node->l->type);
        gen_store_a(fp, node->l->type, RDX, 0);
        break;

    case NOD_ADD_ASSIGN:
        gen_comment(fp, "add-assign");
        gen_address(fp, node->l);
        code2(fp, PUSH, RAX);
        gen_code(fp, node->r);
        code2(fp, POP,  RDX);
        code3(fp, ADD, a_, mem(RDX, 0));
        code3(fp, MOV, mem(RDX, 0), a_);
        break;

    case NOD_SUB_ASSIGN:
        gen_comment(fp, "sub-assign");
        gen_address(fp, node->l);
        code2(fp, PUSH, RAX);
        gen_code(fp, node->r);
        code2(fp, POP,  RDX);
        code3(fp, SUB, a_, mem(RDX, 0));
        code3(fp, MOV, mem(RDX, 0), a_);
        break;

    case NOD_MUL_ASSIGN:
        gen_comment(fp, "mul-assign");
        gen_address(fp, node->l);
        code2(fp, PUSH, RAX);
        gen_code(fp, node->r);
        code2(fp, POP,  RDX);
        code3(fp, IMUL, mem(RDX, 0), a_);
        code3(fp, MOV, a_, mem(RDX, 0));
        break;

    case NOD_DIV_ASSIGN:
        gen_comment(fp, "div-assign");
        gen_address(fp, node->l);
        code2(fp, PUSH, RAX);
        gen_code(fp, node->r);
        code3(fp, MOV, a_, di_);
        code2(fp, POP, RSI);
        code3(fp, MOV, mem(RSI, 0), a_);
        /* rax -> rdx:rax */
        code1(fp, CLTD);
        code2(fp, IDIV, di_);
        code3(fp, MOV, a_, mem(RSI, 0));
        break;

    case NOD_MOD_ASSIGN:
        gen_comment(fp, "mod-assign");
        gen_address(fp, node->l);
        code2(fp, PUSH, RAX);
        gen_code(fp, node->r);
        code3(fp, MOV, a_, di_);
        code2(fp, POP, RSI);
        code3(fp, MOV, mem(RSI, 0), a_);
        gen_div(fp, node, DI_);
        code3(fp, MOV, d_, a_);
        code3(fp, MOV, a_, mem(RSI, 0));
        break;

    case NOD_SHL_ASSIGN:
        gen_comment(fp, "shl-assign");
        gen_address(fp, node->l);
        code2(fp, PUSH, RAX);
        gen_code(fp, node->r);
        code3(fp, MOV, a_, c_);
        code2(fp, POP, RDX);
        code3(fp, MOV, mem(RDX, 0), a_);
        code3(fp, SHL, CL, a_);
        code3(fp, MOV, a_, mem(RDX, 0));
        break;

    case NOD_SHR_ASSIGN:
        gen_comment(fp, "shr-assign");
        gen_address(fp, node->l);
        code2(fp, PUSH, RAX);
        gen_code(fp, node->r);
        code3(fp, MOV, a_, c_);
        code2(fp, POP, RDX);
        code3(fp, MOV, mem(RDX, 0), a_);
        if (is_unsigned(node->type))
            code3(fp, SHR, CL, a_);
        else
            code3(fp, SAR, CL, a_);
        code3(fp, MOV, a_, mem(RDX, 0));
        break;

    case NOD_OR_ASSIGN:
        gen_comment(fp, "or-assign");
        gen_address(fp, node->l);
        code2(fp, PUSH, RAX);
        gen_code(fp, node->r);
        code2(fp, POP, RDX);
        code3(fp, OR, a_, mem(RDX, 0));
        code3(fp, MOV, mem(RDX, 0), a_);
        break;

    case NOD_XOR_ASSIGN:
        gen_comment(fp, "or-assign");
        gen_address(fp, node->l);
        code2(fp, PUSH, RAX);
        gen_code(fp, node->r);
        code2(fp, POP, RDX);
        code3(fp, XOR, a_, mem(RDX, 0));
        code3(fp, MOV, mem(RDX, 0), a_);
        break;

    case NOD_AND_ASSIGN:
        gen_comment(fp, "or-assign");
        gen_address(fp, node->l);
        code2(fp, PUSH, RAX);
        gen_code(fp, node->r);
        code2(fp, POP,  RDX);
        code3(fp, AND, a_, mem(RDX, 0));
        code3(fp, MOV, mem(RDX, 0), a_);
        break;

    case NOD_ADDR:
        gen_address(fp, node->l);
        break;

    case NOD_CAST:
        gen_code(fp, node->r);
        gen_cast(fp, node);
        break;

    case NOD_DEREF:
        gen_code(fp, node->l);
        gen_load(fp, node, mem(RAX, 0), A_);
        break;

    case NOD_NUM:
        code3(fp, MOV, imm(node->ival), a_);
        break;

    case NOD_STRING:
        code3(fp, LEA, symb(STR_LIT_NAME_PREFIX, node->sym->id), RAX);
        break;

    case NOD_SIZEOF:
        code3(fp, MOV, imm(get_size(node->l->type)), EAX);
        break;

    case NOD_ADD:
        gen_code(fp, node->l);
        code2(fp, PUSH, RAX);
        gen_code(fp, node->r);
        code2(fp, POP, RDX);

        /* TODO find the best place to handle array subscript */
        if (is_array(node->l->type) || is_pointer(node->l->type)) {
            const int sz = get_size(underlying(node->l->type));
            code3(fp, IMUL, imm(sz), RAX);
        }

        code3(fp, ADD, d_, a_);
        break;

    case NOD_SUB:
        gen_code(fp, node->l);
        code2(fp, PUSH, RAX);
        gen_code(fp, node->r);
        code3(fp, MOV, a_, d_);
        code2(fp, POP, RAX);
        code3(fp, SUB, d_, a_);
        break;

    case NOD_MUL:
        gen_code(fp, node->l);
        code2(fp, PUSH, RAX);
        gen_code(fp, node->r);
        code2(fp, POP, RDX);
        code3(fp, IMUL, d_, a_);
        break;

    case NOD_DIV:
        gen_code(fp, node->l);
        code2(fp, PUSH, RAX);
        gen_code(fp, node->r);
        code3(fp, MOV, a_, di_);
        code2(fp, POP, RAX);
        /* rax -> rdx:rax */
        code1(fp, CLTD);
        code2(fp, IDIV, di_);
        break;

    case NOD_MOD:
        gen_code(fp, node->l);
        code2(fp, PUSH, RAX);
        gen_code(fp, node->r);
        code3(fp, MOV, a_, di_);
        code2(fp, POP, RAX);
        gen_div(fp, node, DI_);
        code3(fp, MOV, d_, a_);
        break;

    case NOD_SHL:
        gen_code(fp, node->l);
        code2(fp, PUSH, RAX);
        gen_code(fp, node->r);
        code3(fp, MOV, a_, c_);
        code2(fp, POP, RAX);
        code3(fp, SHL, CL, a_);
        break;

    case NOD_SHR:
        gen_code(fp, node->l);
        code2(fp, PUSH, RAX);
        gen_code(fp, node->r);
        code3(fp, MOV, a_, c_);
        code2(fp, POP, RAX);
        if (is_unsigned(node->type))
            code3(fp, SHR, CL, a_);
        else
            code3(fp, SAR, CL, a_);
        break;

    case NOD_OR:
        gen_code(fp, node->l);
        code2(fp, PUSH, RAX);
        gen_code(fp, node->r);
        code2(fp, POP, RDX);
        code3(fp, OR, d_, a_);
        break;

    case NOD_XOR:
        gen_code(fp, node->l);
        code2(fp, PUSH, RAX);
        gen_code(fp, node->r);
        code2(fp, POP, RDX);
        code3(fp, XOR, d_, a_);
        break;

    case NOD_AND:
        gen_code(fp, node->l);
        code2(fp, PUSH, RAX);
        gen_code(fp, node->r);
        code2(fp, POP, RDX);
        code3(fp, AND, d_, a_);
        break;

    case NOD_NOT:
        gen_code(fp, node->l);
        code2(fp, NOT, a_);
        break;

    case NOD_COND:
        /* cond */
        gen_comment(fp, "cond-?");
        tmp = scope;
        scope.curr = next_scope++;

        gen_code(fp, node->l);
        code3(fp, CMP, imm(0), a_);
        code2(fp, JE,  make_label(scope.curr, JMP_ELSE));
        gen_code(fp, node->r);

        scope = tmp;
        break;

    case NOD_COND_THEN:
        /* then */
        gen_comment(fp, "cond-then");
        gen_code(fp, node->l);
        code2(fp, JMP, make_label(scope.curr, JMP_EXIT));
        /* else */
        gen_comment(fp, "cond-else");
        gen_label(fp, scope.curr, JMP_ELSE);
        gen_code(fp, node->r);
        gen_label(fp, scope.curr, JMP_EXIT);
        break;

    case NOD_LOGICAL_OR:
        tmp = scope;
        scope.curr = next_scope++;

        gen_code(fp, node->l);
        code3(fp, CMP, imm(0), a_);
        code2(fp, JNE, make_label(scope.curr, JMP_CONTINUE));
        gen_code(fp, node->r);
        code3(fp, CMP, imm(0), a_);
        code2(fp, JE,  make_label(scope.curr, JMP_EXIT));
        gen_label(fp, scope.curr, JMP_CONTINUE);
        code3(fp, MOV, imm(1), EAX);
        gen_label(fp, scope.curr, JMP_EXIT);

        scope = tmp;
        break;

    case NOD_LOGICAL_AND:
        tmp = scope;
        scope.curr = next_scope++;

        gen_code(fp, node->l);
        code3(fp, CMP, imm(0), a_);
        code2(fp, JE,  make_label(scope.curr, JMP_EXIT));
        gen_code(fp, node->r);
        code3(fp, CMP, imm(0), a_);
        code2(fp, JE,  make_label(scope.curr, JMP_EXIT));
        code3(fp, MOV, imm(1), EAX);
        gen_label(fp, scope.curr, JMP_EXIT);

        scope = tmp;
        break;

    case NOD_LOGICAL_NOT:
        gen_code(fp, node->l);
        code3(fp, CMP, imm(0), a_);
        code2(fp, SETE,  AL);
        code3(fp, MOVZB, AL, a_);
        break;

    case NOD_COMMA:
        gen_code(fp, node->l);
        gen_code(fp, node->r);
        break;

    case NOD_PREINC:
        gen_preincdec(fp, node, ADD);
        break;

    case NOD_PREDEC:
        gen_preincdec(fp, node, SUB);
        break;

    case NOD_POSTINC:
        gen_postincdec(fp, node, ADD);
        break;

    case NOD_POSTDEC:
        gen_postincdec(fp, node, SUB);
        break;

    case NOD_LT:
        gen_relational(fp, node, SETL);
        break;

    case NOD_GT:
        gen_relational(fp, node, SETG);
        break;

    case NOD_LE:
        gen_relational(fp, node, SETLE);
        break;

    case NOD_GE:
        gen_relational(fp, node, SETGE);
        break;

    case NOD_EQ:
        gen_equality(fp, node, SETE);
        break;

    case NOD_NE:
        gen_equality(fp, node, SETNE);
        break;

    default:
        break;
    }
}

static void gen_global_vars(FILE *fp, const struct ast_node *node)
{
    if (!node)
        return;

    if (node->kind == NOD_DECL_IDENT) {
        if (is_global_var(node->sym))
            gen_initializer(fp, node, node->l);
        return;
    }

    gen_global_vars(fp, node->l);
    gen_global_vars(fp, node->r);
}

static void gen_string_literal(FILE *fp, const struct symbol_table *table)
{
    struct symbol *sym;

    for (sym = table->head; sym; sym = sym->next) {
        if (is_string_literal(sym)) {
            gen_string_literal_name(fp, sym->id);
            fprintf(fp, ":\n");
            fprintf(fp, "    .asciz \"");
            print_string_literal(fp, sym->name);
            fprintf(fp, "\"\n\n");
        }
    }
}

void gen_x64(FILE *fp,
        const struct ast_node *tree, const struct symbol_table *table)
{
    if (!att_syntax)
        fprintf(fp, ".intel_syntax noprefix\n");

    fprintf(fp, "    .data\n\n");
    gen_global_vars(fp, tree);

    fprintf(fp, "    .text\n\n");
    gen_string_literal(fp, table);
    gen_code(fp, tree);
}
