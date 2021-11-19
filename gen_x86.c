#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "gen_x86.h"
#include "type.h"

static int att_syntax = 1;

enum data_tag {
    VARI = -1, /* variable based on context */
    BYTE = 0,
    WORD,
    LONG,
    QUAD
};

struct data_spec {
    const char *suffix;
    const char *directive;
    const char *sizename;
};

const struct data_spec data_spec_table[] = {
    {"b", "byte  ptr", "byte"},
    {"w", "word  ptr", "word"},
    {"l", "dword ptr", "long"},
    {"q", "qword ptr", "quad"}
};

static const char *get_data_suffix(int tag)
{
    return data_spec_table[tag].suffix;
}

static const char *get_data_directive(int tag)
{
    return data_spec_table[tag].directive;
}

static const char *get_data_name(int tag)
{
    return data_spec_table[tag].sizename;
}

/* register tables */
static const char *A__[]  = {"al",  "ax", "eax", "rax"};
static const char *B__[]  = {"bl",  "bx", "ebx", "rbx"};
static const char *C__[]  = {"cl",  "cx", "ecx", "rcx"};
static const char *D__[]  = {"dl",  "dx", "edx", "rdx"};
static const char *SI__[] = {"sil", "si", "esi", "rsi"};
static const char *DI__[] = {"dil", "di", "edi", "rdi"};
static const char *IP__[] = {"ipl", "ip", "eip", "rip"};
static const char *BP__[] = {"bpl", "bp", "ebp", "rbp"};
static const char *SP__[] = {"spl", "sp", "esp", "rsp"};
static const char *R8__[] = {"r8b", "r8w", "r8d", "r8"};
static const char *R9__[] = {"r9b", "r9w", "r9d", "r9"};
static const char *R10__[] = {"r10b", "r10w", "r10d", "r10"};
static const char *R11__[] = {"r11b", "r11w", "r11d", "r11"};
static const char **ARG_REG__[] = {DI__, SI__, D__, C__, R8__, R9__};

struct opecode {
    const char *mnemonic;
    int has_suffix;
};

/* opecodes */
const struct opecode MOV_   = {"mov",   1};
const struct opecode ADD_   = {"add",   1};
const struct opecode SUB_   = {"sub",   1};
const struct opecode IMUL_  = {"imul",  1};
const struct opecode DIV_   = {"div",   1};
const struct opecode IDIV_  = {"idiv",  1};
const struct opecode SHL_   = {"shl",   1};
const struct opecode SHR_   = {"shr",   1};
const struct opecode SAR_   = {"sar",   1};
const struct opecode OR_    = {"or",    1};
const struct opecode XOR_   = {"xor",   1};
const struct opecode AND_   = {"and",   1};
const struct opecode NOT_   = {"not",   1};
const struct opecode CMP_   = {"cmp",   1};
const struct opecode POP_   = {"pop",   0};
const struct opecode PUSH_  = {"push",  0};
const struct opecode CALL_  = {"call",  0};
const struct opecode LEA_   = {"lea",   0};
const struct opecode RET_   = {"ret",   0};
const struct opecode MOVSB_ = {"movsb", 1};
const struct opecode MOVSW_ = {"movsw", 1};
const struct opecode MOVSL_ = {"movsl", 1};
const struct opecode MOVZB_ = {"movzb", 1};
const struct opecode MOVZW_ = {"movzw", 1};

const struct opecode JE_    = {"je",  0};
const struct opecode JNE_   = {"jne", 0};
const struct opecode JMP_   = {"jmp", 0};

const struct opecode SETE_  = {"sete",  0};
const struct opecode SETNE_ = {"setne", 0};
const struct opecode SETL_  = {"setl",  0};
const struct opecode SETG_  = {"setg",  0};
const struct opecode SETLE_ = {"setle", 0};
const struct opecode SETGE_ = {"setge", 0};
const struct opecode CLTD_  = {"cltd",  0};
const struct opecode CQTO_  = {"cqto",  0};

enum operand_kind {
    OPR_NONE,
    OPR_REG,
    OPR_ADDR,
    OPR_IMME,
    OPR_LABEL,
    OPR_LABEL__,
    OPR_STR
};

struct operand {
    int kind;
    int data_tag;
    const char **reg_table;
    const char *string;
    long immediate;
    int disp;
    int label_id;
    int block_id;
};
#define INIT_OPERAND {OPR_NONE, VARI}

/* variable name registers */
const struct operand A_  = {OPR_REG, VARI, A__};
const struct operand B_  = {OPR_REG, VARI, B__};
const struct operand C_  = {OPR_REG, VARI, C__};
const struct operand D_  = {OPR_REG, VARI, D__};
const struct operand SI_ = {OPR_REG, VARI, SI__};
const struct operand DI_ = {OPR_REG, VARI, DI__};
const struct operand IP_ = {OPR_REG, VARI, IP__};
const struct operand BP_ = {OPR_REG, VARI, BP__};
const struct operand SP_ = {OPR_REG, VARI, SP__};

/* fixed name registers */
const struct operand AL  = {OPR_REG, BYTE, A__};
const struct operand AX  = {OPR_REG, WORD, A__};
const struct operand EAX = {OPR_REG, LONG, A__};
const struct operand CL  = {OPR_REG, BYTE, C__};

const struct operand RAX = {OPR_REG, QUAD, A__};
const struct operand RCX = {OPR_REG, QUAD, C__};
const struct operand RDX = {OPR_REG, QUAD, D__};
const struct operand RSI = {OPR_REG, QUAD, SI__};
const struct operand RDI = {OPR_REG, QUAD, DI__};
const struct operand RIP = {OPR_REG, QUAD, IP__};
const struct operand RBP = {OPR_REG, QUAD, BP__};
const struct operand RSP = {OPR_REG, QUAD, SP__};
const struct operand R10 = {OPR_REG, QUAD, R10__};
const struct operand R11 = {OPR_REG, QUAD, R11__};

/* 2, 0x8, ... */
struct operand imme(long value)
{
    struct operand o = INIT_OPERAND;
    o.kind = OPR_IMME;
    o.immediate = value;

    return o;
}

/* (base) */
struct operand addr1(struct operand oper)
{
    struct operand o = oper;
    o.kind = OPR_ADDR;
    o.disp = 0;

    return o;
}

/* disp(base) */
struct operand addr2(struct operand oper, int disp)
{
    struct operand o = oper;
    o.kind = OPR_ADDR;
    o.disp = disp;

    return o;
}

/* name(base) */
struct operand addr2_pc_rel(struct operand oper, const char *name, int label_id)
{
    struct operand o = oper;
    o.kind = OPR_ADDR;
    o.string = name;
    o.label_id = label_id;

    return o;
}

/* _main, .L001, ... */
struct operand str(const char *value)
{
    struct operand o = INIT_OPERAND;
    o.kind = OPR_STR;
    o.string = value;

    return o;
}

/* rdi, rsi, ... */
struct operand arg(int index)
{
    struct operand o = INIT_OPERAND;
    o.kind = OPR_REG;
    o.reg_table = ARG_REG__[index];

    return o;
}

/* .LBB1_2, ... */
struct operand label(int block_id, int label_id)
{
    struct operand o = {0};
    o.kind = OPR_LABEL;
    o.block_id = block_id;
    o.label_id = label_id;

    return o;
}

/* _LBB1_2, ... */
struct operand label__(const char *label_str, int label_id)
{
    struct operand o = {0};
    o.kind = OPR_LABEL__;
    o.string = label_str;
    o.label_id = label_id;

    return o;
}

static const char *reg(const struct operand *oper, int tag)
{
    if (oper->data_tag == VARI) {
        return oper->reg_table[tag];
    } else {
        return oper->reg_table[oper->data_tag];
    }
}

static int promote_tag(int tag, const struct operand *oper)
{
    int t;

    if (oper == NULL) {
        return tag;
    }

    /* register that holds address doesn't affect promotion
     * as we don't know the size of data the address points to
     */
    if (oper->kind == OPR_ADDR) {
        return tag;
    }

    t = oper->data_tag;
    return tag > t ? tag : t;
}

static void gen_opecode__(FILE *fp, int tag, const struct opecode *op, int *nchars)
{
    int len = 0;
    const char *sfx = "";

    if (att_syntax) {
        sfx = get_data_suffix(tag);
    }

    if (!op->has_suffix) {
        sfx = "";
    }

    fprintf(fp, "%s%s", op->mnemonic, sfx);

    len = strlen(op->mnemonic) + strlen(sfx);

    *nchars = len;
}

static void gen_pc_rel_addr(FILE *fp, const char *name, int label_id)
{
    if (label_id > 0)
        fprintf(fp, "_%s_%d", name, label_id);
    else
        fprintf(fp, "_%s", name);
}

static void gen_operand__(FILE *fp, int tag, const struct operand *oper)
{
    switch (oper->kind) {

    case OPR_NONE:
        break;

    case OPR_REG:
        if (att_syntax) {
            fprintf(fp, "%%%s", reg(oper, tag));
        } else {
            fprintf(fp, "%s", reg(oper, tag));
        }
        break;

    case OPR_ADDR:
        if (att_syntax) {
            if (oper->string) {
                gen_pc_rel_addr(fp, oper->string, oper->label_id);
                fprintf(fp, "(%%%s)", reg(oper, tag));
            } else
            if (oper->disp != 0) {
                fprintf(fp, "%d(%%%s)", oper->disp, reg(oper, tag));
            } else {
                fprintf(fp, "(%%%s)", reg(oper, tag));
            }
        } else {
            if (oper->disp != 0) {
                fprintf(fp, "%s [%s%+d]",
                    get_data_directive(tag), reg(oper, tag), oper->disp);
            } else {
                fprintf(fp, "%s [%s]", get_data_directive(tag), reg(oper, tag));
            }
        }
        break;

    case OPR_IMME:
        if (att_syntax) {
            fprintf(fp, "$%ld", oper->immediate);
        } else {
            fprintf(fp, "%ld", oper->immediate);
        }
        break;

    case OPR_LABEL:
        fprintf(fp, ".LBB%d_%d", oper->block_id, oper->label_id);
        break;

    case OPR_LABEL__:
        if (oper->label_id == 0)
            fprintf(fp, "_%s(%%rip)", oper->string);
        else
            fprintf(fp, "_%s.%d(%%rip)", oper->string, oper->label_id);
        break;

    case OPR_STR:
        fprintf(fp, "_%s", oper->string);
        break;

    default:
        break;
    }
}

static void code__(FILE *fp, int tag,
        const struct opecode *op, const struct operand *oper1, const struct operand *oper2)
{
    const struct operand *o1 = NULL, *o2 = NULL;
    int nchars = 0;
    int dtag_ = tag;

    dtag_ = promote_tag(dtag_, oper1);
    dtag_ = promote_tag(dtag_, oper2);

    fprintf(fp, "    ");

    gen_opecode__(fp, dtag_, op, &nchars);

    if (oper1 != NULL && oper2 == NULL) {
        o1 = oper1;
    } else {
        if (att_syntax) {
            o1 = oper1;
            o2 = oper2;
        } else {
            o1 = oper2;
            o2 = oper1;
        }
    }

    if (o1) {
        const int max_pad = 7;
        const int pad = nchars > max_pad ? 0 : max_pad - nchars;
        fprintf(fp, "%*s", pad, "");
        gen_operand__(fp, dtag_, o1);
    }

    if (o2) {
        fprintf(fp, ", ");
        gen_operand__(fp, dtag_, o2);
    }

    fprintf(fp, "\n");
}

static int data_tag_(const struct data_type *type)
{
    if (is_char(type))
        return BYTE;
    if (is_short(type))
        return WORD;
    if (is_int(type))
        return LONG;
    if (is_long(type))
        return QUAD;
    if (is_enum(type))
        return LONG;
    return QUAD;
}

static int get_mem_offset(const struct ast_node *node)
{
    return node->sym->mem_offset;
}

static void code1__(FILE *fp, const struct ast_node *node,
        struct opecode op)
{
    const struct opecode o0 = op;
    const int tag = data_tag_(node->type);

    code__(fp, tag, &o0, NULL, NULL);
}

/* because of the return address is already pushed when a fuction starts
 * the rbp % 0x10 should be 0x08 */
static int stack_align = 8;

static void inc_stack_align(int byte)
{
    stack_align += byte;
}

static void dec_stack_align(int byte)
{
    stack_align -= byte;
}

static int need_adjust_stack_align(void)
{
    return stack_align % 16 != 0;
}

static void code2__(FILE *fp, const struct ast_node *node,
        struct opecode op, struct operand oper1)
{
    const struct opecode o0 = op;
    const struct operand o1 = oper1;
    int tag = node ? data_tag_(node->type) : QUAD;

    /* 64 bit mode supports only full register for pop and push */
    if (!strcmp(op.mnemonic, "push") ||
        !strcmp(op.mnemonic, "pop"))
        tag = QUAD;

    if (!strcmp(op.mnemonic, "push"))
        inc_stack_align(8);
    if (!strcmp(op.mnemonic, "pop"))
        dec_stack_align(8);

    code__(fp, tag, &o0, &o1, NULL);
}

static void code3__(FILE *fp, const struct ast_node *node,
        struct opecode op, struct operand oper1, struct operand oper2)
{
    struct opecode o0 = op;
    struct operand o1 = oper1;
    struct operand o2 = oper2;
    int tag = node ? data_tag_(node->type) : QUAD;

    /* this rule comes from x86-64 machine instructions.
     * it depends on the size of register when loading from memory.
     * it is independent of language data types. */
    /* TODO move this to gen_load()? */
    if (!strcmp(op.mnemonic, "mov") &&
        oper1.kind == OPR_ADDR &&
        oper2.kind == OPR_REG)
    {
        switch (tag) {
        case BYTE:
            o0 = is_unsigned(node->type) ? MOVZB_ : MOVSB_;
            o2 = EAX;
            break;
        case WORD:
            o0 = is_unsigned(node->type) ? MOVZW_ : MOVSW_;
            o2 = EAX;
            break;
        default:
            break;
        }
    }

    code__(fp, tag, &o0, &o1, &o2);
}

/* forward declaration */
static void gen_code(FILE *fp, const struct ast_node *node);
static void gen_address(FILE *fp, const struct ast_node *node);
static void gen_assign_struct(FILE *fp, const struct data_type *type);

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

static const struct ast_node *find_node(const struct ast_node *node, int node_kind)
{
    const struct ast_node *found = NULL;

    if (!node)
        return NULL;

    if (node->kind == node_kind)
        return node;

    found = find_node(node->l, node_kind);
    if (found)
        return found;

    found = find_node(node->r, node_kind);
    if (found)
        return found;

    return NULL;
}

static void gen_add_stack_pointer(FILE *fp, int byte)
{
    if (!byte)
        return;
    code3__(fp, NULL, ADD_, imme(byte), RSP);
    inc_stack_align(byte);
}

static void gen_sub_stack_pointer(FILE *fp, int byte)
{
    if (!byte)
        return;
    code3__(fp, NULL, SUB_, imme(byte), RSP);
    dec_stack_align(byte);
}

static void gen_func_param_list_variadic_(FILE *fp)
{
    int i;
    for (i = 0; i < 6; i++) {
        struct ast_node dummy = {0};
        const int disp = -8 * (6 - i);
        code3__(fp, &dummy, MOV_, arg(i), addr2(RBP, disp));
    }
}

static int gen_store_param(FILE *fp, const struct symbol *sym, int stored_regs)
{
    const int size = get_size(sym->type);
    const int N8 = size / 8;
    const int N4 = (size - 8 * N8) / 4;
    int offset = 0;
    int reg = stored_regs;
    int i;

    fprintf(fp, "    movq   %%rbp, %%r10\n");
    fprintf(fp, "    subq   $%d, %%r10\n", sym->mem_offset);

    for (i = 0; i < N8; i++) {
        const char *src_reg = ARG_REG__[reg][QUAD];
        if (offset == 0)
            fprintf(fp, "    movq   %%%s, (%%r10)\n", src_reg);
        else
            fprintf(fp, "    movq   %%%s, %d(%%r10)\n", src_reg, offset);
        reg++;
        offset += 8;
    }
    for (i = 0; i < N4; i++) {
        const char *src_reg = ARG_REG__[reg][LONG];
        if (offset == 0)
            fprintf(fp, "    movl   %%%s, (%%r10)\n", src_reg);
        else
            fprintf(fp, "    movl   %%%s, %d(%%r10)\n", src_reg, offset);
        reg++;
        offset += 4;
    }

    return reg;
}

static void gen_func_param_list_(FILE *fp, const struct symbol *func_sym)
{
    const struct symbol *sym;
    int stored_reg_count = 0;
    int stack_offset = 16; /* from rbp */

    if (is_large_object(func_sym->type)) {
        gen_comment(fp, "save address to returning value");
        code2__(fp, NULL, PUSH_, RDI);
        stored_reg_count++;
    }

    for (sym = first_param(func_sym); sym; sym = next_param(sym)) {
        const int param_size = get_size(sym->type);

        if (is_ellipsis(sym))
            break;

        if (param_size <= 8 && stored_reg_count < 6) {
            /* TODO consider removing dummy by changing node parameter to data_type */
            struct ast_node dummy = {0};
            const int disp = -1 * sym->mem_offset;

            dummy.type = sym->type;
            code3__(fp, &dummy, MOV_, arg(stored_reg_count), addr2(RBP, disp));
            stored_reg_count++;
        }
        else if (param_size <= 16 && stored_reg_count < 5) {
            stored_reg_count = gen_store_param(fp, sym, stored_reg_count);
        }
        else {
            gen_comment(fp, "save rdi and rdx arg");
            code3__(fp, NULL, MOV_, RDI, R10);
            code3__(fp, NULL, MOV_, RDX, R11);

            /* src from stack */
            code3__(fp, NULL, MOV_, RBP, RAX);
            code3__(fp, NULL, ADD_, imme(stack_offset), RAX);
            /* dst from local */
            code3__(fp, NULL, MOV_, RBP, RDX);
            code3__(fp, NULL, SUB_, imme(sym->mem_offset), RDX);
            gen_assign_struct(fp, sym->type);

            gen_comment(fp, "restore rdi and rdx arg");
            code3__(fp, NULL, MOV_, R10, RDI);
            code3__(fp, NULL, MOV_, R11, RDX);

            /* 8 byte align */
            stack_offset += align_to(param_size, 8);
        }
    }
}

static void gen_func_param_list(FILE *fp, const struct ast_node *node)
{
    const struct ast_node *fdecl, *func;

    fdecl = find_node(node, NOD_DECL_FUNC);
    func = find_node(fdecl->l, NOD_DECL_IDENT);

    if (is_variadic(func->sym))
        gen_func_param_list_variadic_(fp);
    else
        gen_func_param_list_(fp, func->sym);
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
    const struct ast_node *fdecl, *func;
    int local_var_size = 0;
    int ret_val_size = 0;

    fdecl = find_node(node->l, NOD_DECL_FUNC);
    func = find_node(fdecl->l, NOD_DECL_IDENT);

    local_var_size = get_mem_offset(func);

    find_max_return_size(node->r, &ret_val_size);
    /* 16 byte align */
    ret_val_size = align_to(ret_val_size, 16);

    local_area_size = local_var_size + ret_val_size;
}

static void gen_func_prologue(FILE *fp, const struct ast_node *node)
{
    const struct ast_node *ddecl = NULL;
    const struct ast_node *ident = NULL;

    /* TODO define find_node_last()? */
    ddecl = find_node(node, NOD_DECL_DIRECT);
    ident = find_node(ddecl->r, NOD_DECL_IDENT);
    /* TODO assert(ident) */

    if (!is_static(ident->sym))
        fprintf(fp, "    .global _%s\n", ident->sym->name);
    fprintf(fp, "_%s:\n", ident->sym->name);
    code2__(fp, ident, PUSH_, RBP);
    code3__(fp, ident, MOV_,  RSP, RBP);
    code3__(fp, ident, SUB_, imme(get_local_area_size()), RSP);
}

static void gen_func_body(FILE *fp, const struct ast_node *node)
{
    const struct ast_node *body = NULL;

    if (0)
        body = find_node(node, NOD_COMPOUND);
        /* TODO assert(body) */
    body = node;

    gen_code(fp, body);
}

static void gen_func_epilogue(FILE *fp, const struct ast_node *node)
{
    code3__(fp, node, MOV_, RBP, RSP);
    code2__(fp, node, POP_, RBP);
    code1__(fp, node, RET_);
}

struct arg_area {
    const struct ast_node *expr;
    int offset;
    int size;
    int pass_by_reg;
};

static void print_arg_area(const struct arg_area *args, int count)
{
    int i;
    for (i = 0; i < count; i++) {
        const struct arg_area *a = &args[i];
        printf("* %2d | size: %d, offset: %d, pass_by_reg: %d\n",
                i, a->size, a->offset, a->pass_by_reg);
    }
}

static void gen_store_arg(FILE *fp, const struct arg_area *arg)
{
    code3__(fp, NULL, MOV_, RSP, RDX);
    code3__(fp, NULL, ADD_, imme(arg->offset), RDX);
    gen_assign_struct(fp, arg->expr->type);
}

static int gen_load_arg(FILE *fp, const struct arg_area *arg, int loaded_regs)
{
    const int size = get_size(arg->expr->type);
    const int N8 = size / 8;
    const int N4 = (size - 8 * N8) / 4;
    int offset = 0;
    int reg = loaded_regs;
    int i;

    for (i = 0; i < N8; i++) {
        const char *dst_reg = ARG_REG__[reg][QUAD];
        if (arg->offset + offset == 0)
            fprintf(fp, "    movq   (%%rsp), %%%s\n", dst_reg);
        else
            fprintf(fp, "    movq   %d(%%rsp), %%%s\n", arg->offset + offset, dst_reg);
        reg++;
        offset += 8;
    }
    for (i = 0; i < N4; i++) {
        const char *dst_reg = ARG_REG__[reg][LONG];
        if (arg->offset + offset == 0)
            fprintf(fp, "    movl   (%%rsp), %%%s\n", dst_reg);
        else
            fprintf(fp, "    movl   %d(%%rsp), %%%s\n", arg->offset + offset, dst_reg);
        reg++;
        offset += 4;
    }

    return reg;
}

static void gen_func_call(FILE *fp, const struct ast_node *node)
{
    /* TODO divide this function */
    const struct symbol *func_sym = node->l->sym;
    struct arg_area *args = NULL;
    int arg_count = node->ival;
    int total_area_size = 0;

    /* use the first register(rdi) for pointer to return value space */
    if (is_large_object(func_sym->type))
        arg_count++;

    /* allocate arg area */
    args = malloc(arg_count * sizeof(struct arg_area));

    /* compute total arg area size */
    {
        const struct ast_node *list = node->r;
        struct arg_area *arg = &args[arg_count - 1];
        int i;

        for (i = 0; i < arg_count; i++) {
            const struct arg_area zero = {NULL, 0, 0, 0};
            args[0] = zero;
        }

        for (; list; list = list->l) {
            const struct ast_node *arg_node = list->r;
            int type_size = 0;

            arg->expr = arg_node->l;
            type_size = get_size(arg->expr->type);

            /* 8 byte align */
            arg->size = align_to(type_size, 8);
            total_area_size += arg->size;

            arg--;
        }
    }

    /* adjust area size */
    {
        const int adjust = need_adjust_stack_align();

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

        for (i = 0; i < arg_count; i++) {
            struct arg_area *arg = &args[i];

            /* skip for return value */
            if (!arg->expr)
                continue;

            if (arg->size == 8 && used_reg_count < 6) {
                reg_offset -= arg->size;

                arg->offset = reg_offset;
                arg->pass_by_reg = 1;

                used_reg_count++;
            }
            else if (arg->size == 16 && used_reg_count < 5) { /* need 2 regs */
                reg_offset -= arg->size;

                arg->offset = reg_offset;
                arg->pass_by_reg = 1;

                used_reg_count += 2;
            }
            else {
                arg->offset = mem_offset;
                arg->pass_by_reg = 0;

                mem_offset += arg->size;
            }
        }
    }

    if (0)
        print_arg_area(args, arg_count);

    {
        int loaded_reg_count = 0;
        int i;

        gen_comment(fp, "allocate arg area");
        gen_sub_stack_pointer(fp, total_area_size);

        /* eval arg expr */
        gen_comment(fp, "store args");
        for (i = 0; i < arg_count; i++) {
            /* skip for return value */
            if (!args[i].expr)
                continue;

            gen_code(fp, args[i].expr);
            if (args[i].size > 8) {
                gen_store_arg(fp, &args[i]);
            } else {
                /* sign extensions on parameter passing */
                const struct data_type *type = args[i].expr->type;
                if (is_char(type)) {
                    if (is_unsigned(type))
                        code3__(fp, node, MOVZB_, AL, RAX);
                    else
                        code3__(fp, node, MOVSB_, AL, RAX);
                }
                else if (is_short(type)) {
                    if (is_unsigned(type))
                        code3__(fp, node, MOVZW_, AX, RAX);
                    else
                        code3__(fp, node, MOVSW_, AX, RAX);
                }
                else if (is_int(type)) {
                    if (is_unsigned(type))
                        fprintf(fp, "    movl   %%eax, %%eax\n");
                    else
                        code3__(fp, node, MOVSL_, EAX, RAX);
                }
                code3__(fp, NULL, MOV_, A_, addr2(RSP, args[i].offset));
            }
        }

        /* load to registers */
        gen_comment(fp, "load args");
        for (i = 0; i < arg_count && loaded_reg_count < 6; i++) {
            struct arg_area *ar = &args[i];

            if (!ar->expr) {
                /* large return value */
                const int offset = -get_local_area_size();
                gen_comment(fp, "load address to returned value");
                code3__(fp, NULL, LEA_, addr2(RBP, offset), arg(0));
                loaded_reg_count++;
                continue;
            }

            if (!ar->pass_by_reg)
                continue;

            if (ar->size > 8) {
                loaded_reg_count = gen_load_arg(fp, ar, loaded_reg_count);
            } else {
                code3__(fp, NULL, MOV_, addr2(RSP, ar->offset),
                        arg(loaded_reg_count));
                loaded_reg_count++;
            }
        }

        /* number of fp */
        if (is_variadic(func_sym))
            fprintf(fp, "    movl   $0, %%eax\n");
        /* TODO fix mov suffix with imme and eax) */
        /* code3__(fp, node, MOV_, imme(0), EAX); */

        /* call */
        gen_comment(fp, "call");
        code2__(fp, node, CALL_, str(func_sym->name));

        gen_comment(fp, "free up arg area");
        gen_add_stack_pointer(fp, total_area_size);
    }

    free(args);

    if (is_medium_object(func_sym->type)) {
        const int offset = -get_local_area_size();

        gen_comment(fp, "store returned value");
        code3__(fp, NULL, MOV_, RAX, addr2(RBP, offset));
        code3__(fp, NULL, MOV_, RDX, addr2(RBP, offset + 8));
        gen_comment(fp, "load address to returned value");
        code3__(fp, NULL, LEA_, addr2(RBP, offset), RAX);
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
        fprintf(fp, "    push   %%rax\n");
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
    fprintf(fp, ".LBB%d_%d:\n", block_id, label_id);
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
            code3__(fp, node, LEA_, addr2_pc_rel(RIP, sym->name, id), RAX);
        } else {
            /* TODO come up with better idea */
            if (!strcmp(sym->name, "__stdinp") ||
                !strcmp(sym->name, "__stdoutp") ||
                !strcmp(sym->name, "__stderrp")) {
                fprintf(fp, "    movq   _%s@GOTPCREL(%%rip), %%rax\n", sym->name);
                code3__(fp, NULL, MOV_, addr1(RAX), RAX);
            } else {
                code3__(fp, node, MOV_, addr2_pc_rel(RIP, sym->name, id), A_);
            }
        }
    }
    else if (is_enumerator(sym)) {
        code3__(fp, node, MOV_, imme(get_mem_offset(node)), A_);
    }
    else {
        const int disp = -get_mem_offset(node);

        if (is_array(node->type)) {
            code3__(fp, node, LEA_, addr2(RBP, disp), A_);
        } else {
            code3__(fp, node, MOV_, addr2(RBP, disp), A_);
        }
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
        code3__(fp, node, LEA_, addr2_pc_rel(RIP, sym->name, id), RAX);
    } else {
        code3__(fp, node, MOV_, BP_, RAX);
        code3__(fp, node, SUB_, imme(get_mem_offset(node)), RAX);
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
            code3__(fp, node, ADD_, imme(disp), RAX);
        }
        break;

    default:
        gen_comment(fp, "not an lvalue");
        break;
    }
}

static void gen_load(FILE *fp, const struct ast_node *node)
{
    /* array objects cannot be loaded in registers, and converted to pointers */
    if (is_array(node->type))
        return;
    /* large struct objects cannot be loaded in registers,
     * and the compiler handle it via pointer */
    if (!is_small_object(node->type))
        return;
    code3__(fp, node, MOV_, addr1(RAX), A_);
}

static void gen_store(FILE *fp, const struct ast_node *node, struct operand addr)
{
    if (is_long(node->type)) {
        if (is_unsigned(node->type))
            code3__(fp, node, MOV_, EAX, RAX);
        else
            code3__(fp, node, MOVSL_, EAX, RAX);
    }

    code3__(fp, node, MOV_, A_, addr1(addr));
}

static void gen_div(FILE *fp, const struct ast_node *node, struct operand divider)
{
    /* rax -> rdx:rax (zero extend) */
    if (is_unsigned(node->type)) {
        code3__(fp, node, XOR_, D_, D_);
        code2__(fp, node, DIV_, divider);
        return;
    }

    /* rax -> rdx:rax (signed extend) */
    if (is_long(node->type))
        code1__(fp, node, CQTO_);
    else
        code1__(fp, node, CLTD_);

    code2__(fp, node, IDIV_, divider);
}

static void gen_preincdec(FILE *fp, const struct ast_node *node, struct opecode op)
{
    int sz = 1;
    if (is_pointer(node->type))
        sz = get_size(underlying(node->type));

    gen_address(fp, node->l);
    code3__(fp, node, op, imme(sz), addr1(RAX));
    code3__(fp, node, MOV_, addr1(RAX), A_);
}

static void gen_postincdec(FILE *fp, const struct ast_node *node, struct opecode op)
{
    int sz = 1;
    if (is_pointer(node->type))
        sz = get_size(underlying(node->type));

    gen_address(fp, node->l);
    code3__(fp, node, MOV_, RAX, RDX);
    code3__(fp, node, MOV_, addr1(RAX), A_);
    code3__(fp, node, op, imme(sz), addr1(RDX));
}

static void gen_relational(FILE *fp, const struct ast_node *node, struct opecode op)
{
    gen_code(fp, node->l);
    code2__(fp, node, PUSH_, RAX);
    gen_code(fp, node->r);
    code3__(fp, node, MOV_, A_, D_);
    code2__(fp, node, POP_, RAX);
    code3__(fp, node, CMP_, D_, A_);
    code2__(fp, node, op, AL);
    code3__(fp, node, MOVZB_, AL, A_);
}

static void gen_equality(FILE *fp, const struct ast_node *node, struct opecode op)
{
    gen_code(fp, node->l);
    code2__(fp, node, PUSH_, RAX);
    gen_code(fp, node->r);
    code2__(fp, node, POP_, RDX);
    code3__(fp, node, CMP_, D_, A_);
    code2__(fp, node, op,   AL);
    code3__(fp, node, MOVZB_, AL, A_);
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
            struct ast_node dummy = {0};
            dummy.type = (struct data_type *) ctrl_type;

            code3__(fp, &dummy, CMP_, imme(node->l->ival), A_);
            code2__(fp, &dummy, JE_,  label(switch_scope, jump_id(node)));
            /* check next statement if it is another case statement */
            gen_switch_table_(fp, node->r, switch_scope, ctrl_type);
        }
        return;

    case NOD_DEFAULT:
        code2__(fp, node, JMP_,  label(switch_scope, jump_id(node)));
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
    code2__(fp, node, JMP_, label(switch_scope, JMP_EXIT));
    gen_comment(fp, "end jump table");
}

static void gen_cast(FILE *fp, const struct ast_node *node)
{
    struct data_type *to;

    if (is_pointer(node->type))
        return;

    to = node->type;

    if (is_char(to)) {
        if (is_unsigned(to))
            code3__(fp, node, MOVZB_, AL, EAX);
        else
            code3__(fp, node, MOVSB_, AL, EAX);
    }
    else if (is_short(to)) {
        if (is_unsigned(to))
            code3__(fp, node, MOVZW_, AX, EAX);
        else
            code3__(fp, node, MOVSW_, AX, EAX);
    }
    else if (is_long(to)) {
        if (is_unsigned(to))
            code3__(fp, node, MOV_,   EAX, RAX);
        else
            code3__(fp, node, MOVSL_, EAX, RAX);
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
        if (offset == 0) {
            fprintf(fp, "    movq   (%%rax), %%rdi\n");
            fprintf(fp, "    movq   %%rdi, (%%rdx)\n");
        } else {
            fprintf(fp, "    movq   %d(%%rax), %%rdi\n", offset);
            fprintf(fp, "    movq   %%rdi, %d(%%rdx)\n", offset);
        }
        offset += 8;
    }
    for (i = 0; i < N4; i++) {
        if (offset == 0) {
            fprintf(fp, "    movl   (%%rax), %%edi\n");
            fprintf(fp, "    movl   %%edi, (%%rdx)\n");
        } else {
            fprintf(fp, "    movl   %d(%%rax), %%edi\n", offset);
            fprintf(fp, "    movl   %%edi, %d(%%rdx)\n", offset);
        }
        offset += 4;
    }
}

static void gen_init_scalar_local(FILE *fp, const struct data_type *type,
        const struct ast_node *ident, int offset, const struct ast_node *expr)
{
    /* TODO fix */
    struct ast_node dummy = {0};
    dummy.type = (struct data_type *) type;

    gen_comment(fp, "local scalar init");

    /* ident */
    gen_address(fp, ident);
    if (offset > 0)
        code3__(fp, ident, ADD_, imme(offset), A_);
    code2__(fp, ident, PUSH_, RAX);

    if (expr) {
        /* init expr */
        gen_code(fp, expr);
        /* assign expr */
        code2__(fp, &dummy, POP_,  RDX);
        gen_comment(fp, "assign object");
        if (is_small_object(type))
            code3__(fp, &dummy, MOV_, A_, addr1(RDX));
        else
            gen_assign_struct(fp, type);
    } else {
        /* assign zero */
        code2__(fp, &dummy, POP_,  RDX);
        code3__(fp, &dummy, MOV_, imme(0), addr1(RAX));
    }
}

struct memory_byte {
    const struct ast_node *init;
    const struct data_type *type;
    int is_written;
    int written_size;
};

struct object_byte {
    struct memory_byte *bytes;
    struct symbol *sym;
    int size;
};

static void print_object(struct object_byte *obj)
{
    int i;

    printf("%s %s:\n", type_name_of(obj->sym->type), obj->sym->name);
    for (i = 0; i < obj->size; i++) {
        printf("    [%04d] is_written: %d written_size: %d init: %p\n",
                i,
                obj->bytes[i].is_written,
                obj->bytes[i].written_size,
                (void *) obj->bytes[i].init);
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
        const struct symbol *sym;

        for (sym = first_member(type->sym); sym; sym = next_member(sym))
            zero_clear_bytes(bytes + sym->mem_offset, sym->type);
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

    obj->bytes = (struct memory_byte *) calloc(size, sizeof(struct memory_byte));
    obj->size = size;
    obj->sym = ident->sym;

    zero_clear_bytes(obj->bytes, obj->sym->type);
}

static void free_object_byte(struct object_byte *obj)
{
    struct object_byte o = {0};
    free(obj->bytes);
    *obj = o;
}

static void assign_init(struct memory_byte *base,
        const struct data_type *type, const struct ast_node *node)
{
    if (!node)
        return;

    switch (node->kind) {

    case NOD_INIT:
        {
            /* move cursor by designator */
            const int offset = node->ival;

            assign_init(base, type, node->l);
            assign_init(base + offset, underlying(type), node->r);
        }
        break;

    case NOD_LIST:
        /* pass to the next initializer */
        assign_init(base, type, node->l);
        assign_init(base, type, node->r);
        break;

    default:
        if (is_struct(node->type)) {
            /* init by struct object */
            const int size = get_size(node->type);
            int i;

            base->init = node;

            for (i = 0; i < size; i++) {
                /* override byte properties as if struct object is a scaler value */
                base[i].is_written = 1;
                base[i].written_size = (i == 0) ? size : 0;
                base[i].type = node->type;
            }
        } else {
            /* assign initializer to byte */
            const int size = base->written_size;
            int i;

            base->init = node;

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
    const int tag = data_tag_(type);
    const char *szname = get_data_name(tag);

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
                gen_pc_rel_addr(fp, sym->name, sym->id);
            else
                gen_pc_rel_addr(fp, sym->name, -1);
            fprintf(fp, "\n");
        }
        break;

    case NOD_ADDR:
        {
            /* TODO make function taking sym */
            const struct symbol *sym = expr->l->sym;
            fprintf(fp, "    .%s ", szname);
            if (is_static(sym))
                gen_pc_rel_addr(fp, sym->name, sym->id);
            else
                gen_pc_rel_addr(fp, sym->name, -1);
            fprintf(fp, "\n");
        }
        break;

    case NOD_CAST:
        /* TODO come up better way to handle scalar universaly */
        gen_init_scalar_global(fp, type, expr->r);
        break;

    case NOD_STRING:
        fprintf(fp, "    .%s ", szname);
        fprintf(fp, "_L.str.%d\n", expr->sym->id);
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
            gen_pc_rel_addr(fp, sym->name, id);
            fprintf(fp, "\n");
        }
        gen_pc_rel_addr(fp, sym->name, id);
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
        gen_object_byte(fp, &obj);
    }
    else if (is_local_var(ident->sym)) {
        int i;

        for (i = 0; i < obj.size; i++) {
            const struct memory_byte *byte = &obj.bytes[i];

            if (byte->written_size > 0)
                gen_init_scalar_local(fp, byte->type, ident, i, byte->init);
        }
    }

    free_object_byte(&obj);
}

static void gen_initializer_global(FILE *fp, const struct ast_node *node)
{
    const struct ast_node *ident;

    if (!node || node->kind != NOD_DECL_INIT)
        return;

    ident = find_node(node->l, NOD_DECL_IDENT);

    if (ident && is_global_var(ident->sym))
        gen_initializer(fp, ident, node->r);
}

static void gen_initializer_local(FILE *fp, const struct ast_node *node)
{
    const struct ast_node *ident;

    if (!node || !node->r || node->kind != NOD_DECL_INIT)
        return;

    ident = find_node(node->l, NOD_DECL_IDENT);

    if (ident && is_local_var(ident->sym))
        gen_initializer(fp, ident, node->r);
}

static void gen_code(FILE *fp, const struct ast_node *node)
{
    static struct jump_scope scope = {0};
    struct jump_scope tmp = {0};
    static int next_scope = 0;

    if (node == NULL)
        return;

    switch (node->kind) {

    case NOD_LIST:
    case NOD_COMPOUND:
    case NOD_DECL:
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
        code3__(fp, node, CMP_, imme(0), A_);
        code2__(fp, node, JE_,  label(scope.curr, JMP_EXIT));
        break;

    case NOD_FOR_BODY_POST:
        /* body */
        gen_comment(fp, "for-body");
        gen_code(fp, node->l);
        /* post */
        gen_comment(fp, "for-post");
        gen_label(fp, scope.curr, JMP_CONTINUE);
        gen_code(fp, node->r);
        code2__(fp, node, JMP_, label(scope.curr, JMP_ENTER));
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
        code3__(fp, node, CMP_, imme(0), A_);
        code2__(fp, node, JE_,  label(scope.curr, JMP_EXIT));
        gen_comment(fp, "while-body");
        gen_code(fp, node->r);
        code2__(fp, node, JMP_, label(scope.curr, JMP_CONTINUE));
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
        code3__(fp, node, CMP_, imme(0), A_);
        code2__(fp, node, JE_,  label(scope.curr, JMP_EXIT));
        code2__(fp, node, JMP_, label(scope.curr, JMP_ENTER));
        gen_label(fp, scope.curr, JMP_EXIT);

        scope = tmp;
        break;

    case NOD_IF:
        /* if */
        tmp = scope;
        scope.curr = next_scope++;

        gen_comment(fp, "if-cond");
        gen_code(fp, node->l);
        code3__(fp, node, CMP_, imme(0), A_);
        code2__(fp, node, JE_,  label(scope.curr, JMP_ELSE));
        gen_code(fp, node->r);

        scope = tmp;
        break;

    case NOD_IF_THEN:
        /* then */
        gen_comment(fp, "if-then");
        gen_code(fp, node->l);
        code2__(fp, node, JMP_, label(scope.curr, JMP_EXIT));
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
            code3__(fp, NULL, MOV_, RAX, RSI);
            code3__(fp, NULL, MOV_, addr1(RSI), A_);
            code3__(fp, NULL, MOV_, addr2(RSI, 8), D_);
        }
        else if (is_large_object(node->type)) {
            gen_comment(fp, "fill returning value");
            code2__(fp, node, POP_, RDX);
            gen_assign_struct(fp, node->type);
            gen_comment(fp, "load address to returning value");
            code3__(fp, NULL, MOV_, RDX, RAX);
        }

        code2__(fp, node, JMP_, label(scope.func, JMP_RETURN));
        break;

    case NOD_BREAK:
        code2__(fp, node, JMP_, label(scope.brk, JMP_EXIT));
        break;

    case NOD_CONTINUE:
        code2__(fp, node, JMP_, label(scope.conti, JMP_CONTINUE));
        break;

    case NOD_LABEL:
        gen_label(fp, scope.func, jump_id(node->l));
        gen_code(fp, node->r);
        break;

    case NOD_GOTO:
        code2__(fp, node, JMP_, label(scope.func, jump_id(node->l)));
        break;

    case NOD_IDENT:
        if (is_small_object(node->type))
            gen_ident(fp, node);
        else
            gen_address(fp, node);
        break;

    case NOD_DECL_INIT:
        gen_initializer_local(fp, node);
        break;

    case NOD_STRUCT_REF:
        gen_address(fp, node);
        gen_load(fp, node);
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
        code2__(fp, node, PUSH_, RAX);
        gen_code(fp, node->r);
        code2__(fp, node, POP_,  RDX);

        /* TODO consider adding cast explicitly */
        if (0)
            gen_store(fp, node, RDX);

        if (is_long(node->type) && !is_long(node->r->type)) {
            if (is_unsigned(node->type))
                code3__(fp, node, MOV_, EAX, RAX);
            else
                code3__(fp, node, MOVSL_, EAX, RAX);
        }

        if (is_small_object(node->type))
            code3__(fp, node, MOV_, A_, addr1(RDX));
        else
            gen_assign_struct(fp, node->type);

        break;

    case NOD_ADD_ASSIGN:
        gen_comment(fp, "add-assign");
        gen_address(fp, node->l);
        code2__(fp, node, PUSH_, RAX);
        gen_code(fp, node->r);
        code2__(fp, node, POP_,  RDX);
        code3__(fp, node, ADD_, A_, addr1(RDX));
        code3__(fp, node, MOV_, addr1(RDX), A_);
        break;

    case NOD_SUB_ASSIGN:
        gen_comment(fp, "sub-assign");
        gen_address(fp, node->l);
        code2__(fp, node, PUSH_, RAX);
        gen_code(fp, node->r);
        code2__(fp, node, POP_,  RDX);
        code3__(fp, node, SUB_, A_, addr1(RDX));
        code3__(fp, node, MOV_, addr1(RDX), A_);
        break;

    case NOD_MUL_ASSIGN:
        gen_comment(fp, "mul-assign");
        gen_address(fp, node->l);
        code2__(fp, node, PUSH_, RAX);
        gen_code(fp, node->r);
        code2__(fp, node, POP_,  RDX);
        code3__(fp, node, IMUL_, addr1(RDX), A_);
        code3__(fp, node, MOV_, A_, addr1(RDX));
        break;

    case NOD_DIV_ASSIGN:
        gen_comment(fp, "div-assign");
        gen_address(fp, node->l);
        code2__(fp, node, PUSH_, RAX);
        gen_code(fp, node->r);
        code3__(fp, node, MOV_, A_, DI_);
        code2__(fp, node, POP_, RSI);
        code3__(fp, node, MOV_, addr1(RSI), A_);
        code1__(fp, node, CLTD_); /* rax -> rdx:rax */
        code2__(fp, node, IDIV_, DI_);
        code3__(fp, node, MOV_, A_, addr1(RSI));
        break;

    case NOD_MOD_ASSIGN:
        gen_comment(fp, "mod-assign");
        gen_address(fp, node->l);
        code2__(fp, node, PUSH_, RAX);
        gen_code(fp, node->r);
        code3__(fp, node, MOV_, A_, DI_);
        code2__(fp, node, POP_, RSI);
        code3__(fp, node, MOV_, addr1(RSI), A_);
        gen_div(fp, node, DI_);
        code3__(fp, node, MOV_, D_, A_);
        code3__(fp, node, MOV_, A_, addr1(RSI));
        break;

    case NOD_SHL_ASSIGN:
        gen_comment(fp, "shl-assign");
        gen_address(fp, node->l);
        code2__(fp, node, PUSH_, RAX);
        gen_code(fp, node->r);
        code3__(fp, node, MOV_, A_, C_);
        code2__(fp, node, POP_,  RDX);
        code3__(fp, node, SHL_, CL, addr1(RDX));
        code3__(fp, node, MOV_, addr1(RDX), A_);
        break;

    case NOD_SHR_ASSIGN:
        gen_comment(fp, "shr-assign");
        gen_address(fp, node->l);
        code2__(fp, node, PUSH_, RAX);
        gen_code(fp, node->r);
        code3__(fp, node, MOV_, A_, C_);
        code2__(fp, node, POP_,  RDX);
        if (is_unsigned(node->type))
            code3__(fp, node, SHR_, CL, addr1(RDX));
        else
            code3__(fp, node, SAR_, CL, addr1(RDX));
        code3__(fp, node, MOV_, addr1(RDX), A_);
        break;

    case NOD_OR_ASSIGN:
        gen_comment(fp, "or-assign");
        gen_address(fp, node->l);
        code2__(fp, node, PUSH_, RAX);
        gen_code(fp, node->r);
        code2__(fp, node, POP_,  RDX);
        code3__(fp, node, OR_, A_, addr1(RDX));
        code3__(fp, node, MOV_, addr1(RDX), A_);
        break;

    case NOD_XOR_ASSIGN:
        gen_comment(fp, "or-assign");
        gen_address(fp, node->l);
        code2__(fp, node, PUSH_, RAX);
        gen_code(fp, node->r);
        code2__(fp, node, POP_,  RDX);
        code3__(fp, node, XOR_, A_, addr1(RDX));
        code3__(fp, node, MOV_, addr1(RDX), A_);
        break;

    case NOD_AND_ASSIGN:
        gen_comment(fp, "or-assign");
        gen_address(fp, node->l);
        code2__(fp, node, PUSH_, RAX);
        gen_code(fp, node->r);
        code2__(fp, node, POP_,  RDX);
        code3__(fp, node, AND_, A_, addr1(RDX));
        code3__(fp, node, MOV_, addr1(RDX), A_);
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
        gen_load(fp, node);
        break;

    case NOD_NUM:
        code3__(fp, node, MOV_, imme(node->ival), A_);
        break;

    case NOD_STRING:
        code3__(fp, node, LEA_, label__("L.str", node->sym->id), A_);
        break;

    case NOD_SIZEOF:
        code3__(fp, node, MOV_, imme(get_size(node->l->type)), A_);
        break;

    case NOD_ADD:
        gen_code(fp, node->l);
        code2__(fp, node, PUSH_, RAX);
        gen_code(fp, node->r);
        code2__(fp, node, POP_, RDX);

        /* TODO find the best place to handle array subscript */
        if (is_array(node->l->type) || is_pointer(node->l->type)) {
            const int sz = get_size(underlying(node->l->type));
            code3__(fp, node, IMUL_, imme(sz), RAX);
        }

        code3__(fp, node, ADD_, D_, A_);
        break;

    case NOD_SUB:
        gen_code(fp, node->l);
        code2__(fp, node, PUSH_, RAX);
        gen_code(fp, node->r);
        code3__(fp, node, MOV_, A_, D_);
        code2__(fp, node, POP_, RAX);
        code3__(fp, node, SUB_, D_, A_);
        break;

    case NOD_MUL:
        gen_code(fp, node->l);
        code2__(fp, node, PUSH_, RAX);
        gen_code(fp, node->r);
        code2__(fp, node, POP_,  RDX);
        code3__(fp, node, IMUL_, D_, A_);
        break;

    case NOD_DIV:
        gen_code(fp, node->l);
        code2__(fp, node, PUSH_, RAX);
        gen_code(fp, node->r);
        code3__(fp, node, MOV_, A_, DI_);
        code2__(fp, node, POP_, RAX);
        code1__(fp, node, CLTD_); /* rax -> rdx:rax */
        code2__(fp, node, IDIV_, DI_);
        break;

    case NOD_MOD:
        gen_code(fp, node->l);
        code2__(fp, node, PUSH_, RAX);
        gen_code(fp, node->r);
        code3__(fp, node, MOV_, A_, DI_);
        code2__(fp, node, POP_, RAX);
        gen_div(fp, node, DI_);
        code3__(fp, node, MOV_, D_, A_);
        break;

    case NOD_SHL:
        gen_code(fp, node->l);
        code2__(fp, node, PUSH_, RAX);
        gen_code(fp, node->r);
        code3__(fp, node, MOV_, A_, C_);
        code2__(fp, node, POP_,  RAX);
        code3__(fp, node, SHL_, CL, A_);
        break;

    case NOD_SHR:
        gen_code(fp, node->l);
        code2__(fp, node, PUSH_, RAX);
        gen_code(fp, node->r);
        code3__(fp, node, MOV_, A_, C_);
        code2__(fp, node, POP_,  RAX);
        if (is_unsigned(node->type))
            code3__(fp, node, SHR_, CL, A_);
        else
            code3__(fp, node, SAR_, CL, A_);
        break;

    case NOD_OR:
        gen_code(fp, node->l);
        code2__(fp, node, PUSH_, RAX);
        gen_code(fp, node->r);
        code2__(fp, node, POP_, RDX);
        code3__(fp, node, OR_, D_, A_);
        break;

    case NOD_XOR:
        gen_code(fp, node->l);
        code2__(fp, node, PUSH_, RAX);
        gen_code(fp, node->r);
        code2__(fp, node, POP_, RDX);
        code3__(fp, node, XOR_, D_, A_);
        break;

    case NOD_AND:
        gen_code(fp, node->l);
        code2__(fp, node, PUSH_, RAX);
        gen_code(fp, node->r);
        code2__(fp, node, POP_, RDX);
        code3__(fp, node, AND_, D_, A_);
        break;

    case NOD_NOT:
        gen_code(fp, node->l);
        code2__(fp, node, NOT_, A_);
        break;

    case NOD_COND:
        /* cond */
        gen_comment(fp, "cond-?");
        tmp = scope;
        scope.curr = next_scope++;

        gen_code(fp, node->l);
        code3__(fp, node, CMP_, imme(0), A_);
        code2__(fp, node, JE_,  label(scope.curr, JMP_ELSE));
        gen_code(fp, node->r);

        scope = tmp;
        break;

    case NOD_COND_THEN:
        /* then */
        gen_comment(fp, "cond-then");
        gen_code(fp, node->l);
        code2__(fp, node, JMP_, label(scope.curr, JMP_EXIT));
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
        code3__(fp, node, CMP_, imme(0), A_);
        code2__(fp, node, JNE_, label(scope.curr, JMP_CONTINUE));
        gen_code(fp, node->r);
        code3__(fp, node, CMP_, imme(0), A_);
        code2__(fp, node, JE_,  label(scope.curr, JMP_EXIT));
        gen_label(fp, scope.curr, JMP_CONTINUE);
        code3__(fp, node, MOV_, imme(1), A_);
        gen_label(fp, scope.curr, JMP_EXIT);

        scope = tmp;
        break;

    case NOD_LOGICAL_AND:
        tmp = scope;
        scope.curr = next_scope++;

        gen_code(fp, node->l);
        code3__(fp, node, CMP_, imme(0), A_);
        code2__(fp, node, JE_,  label(scope.curr, JMP_EXIT));
        gen_code(fp, node->r);
        code3__(fp, node, CMP_, imme(0), A_);
        code2__(fp, node, JE_,  label(scope.curr, JMP_EXIT));
        code3__(fp, node, MOV_, imme(1), A_);
        gen_label(fp, scope.curr, JMP_EXIT);

        scope = tmp;
        break;

    case NOD_LOGICAL_NOT:
        gen_code(fp, node->l);
        code3__(fp, node, CMP_, imme(0), A_);
        code2__(fp, node, SETE_,  AL);
        code3__(fp, node, MOVZB_, AL, A_);
        break;

    case NOD_PREINC:
        gen_preincdec(fp, node, ADD_);
        break;

    case NOD_PREDEC:
        gen_preincdec(fp, node, SUB_);
        break;

    case NOD_POSTINC:
        gen_postincdec(fp, node, ADD_);
        break;

    case NOD_POSTDEC:
        gen_postincdec(fp, node, SUB_);
        break;

    case NOD_LT:
        gen_relational(fp, node, SETL_);
        break;

    case NOD_GT:
        gen_relational(fp, node, SETG_);
        break;

    case NOD_LE:
        gen_relational(fp, node, SETLE_);
        break;

    case NOD_GE:
        gen_relational(fp, node, SETGE_);
        break;

    case NOD_EQ:
        gen_equality(fp, node, SETE_);
        break;

    case NOD_NE:
        gen_equality(fp, node, SETNE_);
        break;

    default:
        break;
    }
}

static void gen_global_vars(FILE *fp, const struct ast_node *node)
{
    if (!node)
        return;

    if (node->kind == NOD_DECL_INIT) {
        gen_initializer_global(fp, node);
        return;
    }

    gen_global_vars(fp, node->l);
    gen_global_vars(fp, node->r);
}

static int char_to_escape_sequence(int ch, char *es)
{
    int es1 = '\0';

    if (!es)
        return 0;

    switch (ch) {
    case '\0': es1 = '0';  break;
    case '\\': es1 = '\\'; break;
    case '\'': es1 = '\''; break;
    case '"':  es1 = '"';  break;
    case '\a': es1 = 'a';  break;
    case '\b': es1 = 'b';  break;
    case '\f': es1 = 'f';  break;
    case '\n': es1 = 'n';  break;
    case '\r': es1 = 'r';  break;
    case '\t': es1 = 't';  break;
    case '\v': es1 = 'v';  break;
    default:
       return 0;
    }

    es[0] = '\\';
    es[1] = es1;
    es[2] = '\0';

    return 1;
}

static void print_string_literal(FILE *fp, const char *src)
{
    const char *s = src;
    char es[4] = {'\0'};

    for (; *s; s++) {
        if (iscntrl(*s))
            fprintf(fp, "\\%03o", *s);
        else if (char_to_escape_sequence(*s, es) && *s != '\'')
            fprintf(fp, "%s", es);
        else
            fprintf(fp, "%c", *s);
    }
}

static void gen_string_literal(FILE *fp, const struct symbol_table *table)
{
    struct symbol *sym;

    for (sym = table->head; sym; sym = sym->next) {
        if (is_string_literal(sym)) {
            fprintf(fp, "_L.str.%d:\n", sym->id);
            fprintf(fp, "    .asciz \"");
            print_string_literal(fp, sym->name);
            fprintf(fp, "\"\n\n");
        }
    }
}

void gen_x86(FILE *fp,
        const struct ast_node *tree, const struct symbol_table *table)
{
    if (!att_syntax) {
        fprintf(fp, ".intel_syntax noprefix\n");
    }

    fprintf(fp, "    .data\n\n");
    gen_string_literal(fp, table);
    gen_global_vars(fp, tree);

    fprintf(fp, "    .text\n\n");
    gen_code(fp, tree);
}
