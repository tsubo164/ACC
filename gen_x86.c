#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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
const struct opecode IDIV_  = {"idiv",  1};
const struct opecode CMP_   = {"cmp",   1};
const struct opecode POP_   = {"pop",   0};
const struct opecode PUSH_  = {"push",  0};
const struct opecode CALL_  = {"call",  0};
const struct opecode LEA_   = {"lea",   0};
const struct opecode RET_   = {"ret",   0};
const struct opecode MOVSB_ = {"movsb", 1};
const struct opecode MOVZB_ = {"movzb", 1};

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
const struct operand EAX = {OPR_REG, LONG, A__};

const struct operand RAX = {OPR_REG, QUAD, A__};
const struct operand RDX = {OPR_REG, QUAD, D__};
const struct operand RSI = {OPR_REG, QUAD, SI__};
const struct operand RIP = {OPR_REG, QUAD, IP__};
const struct operand RBP = {OPR_REG, QUAD, BP__};
const struct operand RSP = {OPR_REG, QUAD, SP__};

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
                fprintf(fp, "%+d(%%%s)", oper->disp, reg(oper, tag));
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
    if (is_int(type))
        return LONG;
    if (is_type_name(type))
        return data_tag_(original_const(type));
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

static void code2__(FILE *fp, const struct ast_node *node,
        struct opecode op, struct operand oper1)
{
    const struct opecode o0 = op;
    const struct operand o1 = oper1;
    int tag = data_tag_(node->type);

    /* 64 bit mode supports only full register for pop and push */
    if (!strcmp(op.mnemonic, "push") ||
        !strcmp(op.mnemonic, "pop"))
        tag = QUAD;

    code__(fp, tag, &o0, &o1, NULL);
}

static void code3__(FILE *fp, const struct ast_node *node,
        struct opecode op, struct operand oper1, struct operand oper2)
{
    struct opecode o0 = op;
    struct operand o1 = oper1;
    struct operand o2 = oper2;
    int tag = data_tag_(node->type);

    /* this rule comes from x86-64 machine instructions.
     * it depends on the size of register when loading from memory.
     * it is independent of language data types. */
    if (!strcmp(op.mnemonic, "mov") &&
        oper1.kind == OPR_ADDR &&
        oper2.kind == OPR_REG)
    {
        switch (tag) {
        case BYTE:
            o0 = MOVSB_;
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

static int gen_one_param(FILE *fp, const struct ast_node *node, int reg_index)
{
    int max_index = 0;

    if (!node)
        return reg_index;

    max_index = gen_one_param(fp, node->l, reg_index);
    max_index = gen_one_param(fp, node->r, max_index);

    if (node->kind == NOD_DECL_IDENT) {
        const int disp = -get_mem_offset(node);

        code3__(fp, node, MOV_, arg(max_index), addr2(RBP, disp));
        return max_index + 1;
    } else {
        return max_index;
    }
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

static void gen_func_param_list(FILE *fp, const struct ast_node *node)
{
    const struct ast_node *fdecl;
    fdecl = find_node(node, NOD_DECL_FUNC);
    gen_one_param(fp, fdecl->r, 0);
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
    code3__(fp, ident, SUB_, imme(get_mem_offset(ident)), RSP);
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

static void gen_func_call(FILE *fp, const struct ast_node *node)
{
    static int reg_count = 0;
    int i;

    if (!node)
        return;

    switch (node->kind) {

    case NOD_CALL:
        reg_count = 0;
        /* push args */
        gen_func_call(fp, node->r);
        /* pop args */
        for (i = 0; i < reg_count; i++)
            code2__(fp, node, POP_, arg(i));
        /* call */
        code2__(fp, node, CALL_, str(node->l->sym->name));
        return;

    case NOD_ARG:
        /* push args */
        gen_code(fp, node->l);
        code2__(fp, node, PUSH_, RAX);
        reg_count++;
        return;

    default:
        /* walk tree from the rightmost arg */
        gen_func_call(fp, node->r);
        gen_func_call(fp, node->l);
        return;
    }
}

static void gen_comment(FILE *fp, const char *cmt)
{
    fprintf(fp, "## %s\n", cmt);
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
        /*
        code3__(fp, node, MOV_, addr2_pc_rel(RIP, sym->name, id), RAX);
        */
        if (is_array(node->type)) {
            code3__(fp, node, LEA_, addr2_pc_rel(RIP, sym->name, id), RAX);
        } else {
            code3__(fp, node, MOV_, addr2_pc_rel(RIP, sym->name, id), RAX);
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

static void gen_lvalue(FILE *fp, const struct ast_node *node)
{
    if (node == NULL) {
        return;
    }

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
            gen_lvalue(fp, node->l);
            code3__(fp, node, ADD_, imme(disp), RAX);
        }
        break;

    default:
        gen_comment(fp, "not an lvalue");
        break;
    }
}

static void gen_preincdec(FILE *fp, const struct ast_node *node, struct opecode op)
{
    int sz = 1;
    if (is_pointer(node->type))
        sz = get_size(underlying(node->type));

    gen_lvalue(fp, node->l);
    code3__(fp, node, op, imme(sz), addr1(RAX));
    code3__(fp, node, MOV_, addr1(RAX), A_);
}

static void gen_postincdec(FILE *fp, const struct ast_node *node, struct opecode op)
{
    int sz = 1;
    if (is_pointer(node->type))
        sz = get_size(underlying(node->type));

    gen_lvalue(fp, node->l);
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

static void gen_switch_table(FILE *fp, const struct ast_node *node, int switch_scope)
{
    if (!node)
        return;

    switch (node->kind) {

    case NOD_CASE:
        code3__(fp, node, CMP_, imme(node->l->ival), A_);
        code2__(fp, node, JE_,  label(switch_scope, jump_id(node)));
        return;

    case NOD_DEFAULT:
        code2__(fp, node, JE_,  label(switch_scope, jump_id(node)));
        return;

    default:
        break;
    }

    gen_switch_table(fp, node->l, switch_scope);
    gen_switch_table(fp, node->r, switch_scope);
}

static void gen_init_scalar_local(FILE *fp, const struct data_type *type,
        const struct ast_node *ident, int offset, const struct ast_node *expr)
{
    /* TODO fix */
    struct ast_node dummy = {0};
    dummy.type = (struct data_type *) type;

    gen_comment(fp, "local scalar init");

    /* ident */
    gen_lvalue(fp, ident);
    if (offset > 0)
        code3__(fp, ident, ADD_, imme(offset), A_);
    code2__(fp, ident, PUSH_, RAX);

    if (expr) {
        /* init expr */
        gen_code(fp, expr);
        /* assign expr */
        code2__(fp, &dummy, POP_,  RDX);
        code3__(fp, &dummy, MOV_, A_, addr1(RDX));
    } else {
        /* assign zero */
        code2__(fp, &dummy, POP_,  RDX);
        code3__(fp, &dummy, MOV_, imme(0), addr1(RAX));
    }
}

static void gen_init_scalar_global(FILE *fp, const struct data_type *type,
        const struct ast_node *ident, int offset, const struct ast_node *expr)
{

    if (offset == 0) {
        struct symbol *sym = ident->sym;
        int id = sym->id;

        if (!is_static(sym)) {
            id = -1;
            fprintf(fp, "    .global ");
            gen_pc_rel_addr(fp, sym->name, id);
            fprintf(fp, "\n");
        }
        gen_pc_rel_addr(fp, sym->name, id);
        fprintf(fp, ":\n");
    }
    {
        int tag;
        const char *szname;
        int val;

        if (expr) {
            tag = data_tag_(type);
            szname = data_spec_table[tag].sizename;
            val = expr->ival;
        } else {
            tag = data_tag_(type);
            szname = data_spec_table[tag].sizename;
            val = 0;
        }
        fprintf(fp, "    .%s %d\n", szname, val);
    }
}

static void gen_init_scalar(FILE *fp, const struct data_type *type,
        const struct ast_node *ident, int offset, const struct ast_node *expr)
{
    if (is_local_var(ident->sym))
        gen_init_scalar_local(fp, type, ident, offset, expr);
    if (is_global_var(ident->sym) && !is_extern(ident->sym))
        gen_init_scalar_global(fp, type, ident, offset, expr);
}

/* TODO may be better to create init table first to check if the initializers are
 * specified or not by looking it up during initialization. */
static void gen_zero_elements(FILE *fp, const struct ast_node *ident,
        const struct data_type *type, int base, int start, int end)
{
    int i;

    for (i = start; i < end; i++) {
        const int offset = base + i * get_size(type);

        if (is_array(type)) {
            const int base_ = offset;
            const int start_ = 0;
            const int end_ = get_array_length(type);

            gen_zero_elements(fp, ident, underlying(type), base_, start_, end_);
        } else {
            gen_init_scalar(fp, type, ident, offset, NULL);
        }
    }
}

/* TODO may be better to create init table first to check if the initializers are
 * specified or not by looking it up during initialization. */
static void gen_zero_members(FILE *fp, const struct ast_node *ident,
        int base, const struct symbol *start)
{
    const struct symbol *sym = start;
    const int scope = start->scope_level;

    for (sym = start; sym; sym = sym->next) {
        const int offset = base + sym->mem_offset;

        if (sym->kind == SYM_SCOPE_END && sym->scope_level == scope)
            break;

        if (/*is_array(type)*/0) {
            /*
            const int base_ = offset;
            const int start_ = 0;
            const int end_ = get_array_length(type);

            gen_zero_elements(fp, ident, underlying(type), base_, start_, end_);
            */
        } else {
            gen_init_scalar(fp, sym->type, ident, offset, NULL);
        }
    }
}

static void gen_init_array(FILE *fp, const struct ast_node *node,
        const struct ast_node *ident, const struct data_type *type)
{
    static int base = 0;
    int index = 0;

    if (!node)
        return;

    index = node->ival;

    switch (node->kind) {

    case NOD_INIT:
        gen_init_array(fp, node->l, ident, type);

        if (is_array(type)) {
            gen_init_array(fp, node->r, ident, type);
        }else {
            const int offset = base + index * get_size(type);
            gen_init_scalar(fp, type, ident, offset, node->r);
        }
        break;

    case NOD_INIT_LIST:
        {
            int tmp;

            base = index * get_size(type);

            tmp = base;
            gen_init_array(fp, node->l, ident, underlying(type));
            base = tmp;
        }
        {
            /* initialize unspecified elements */
            const int start = node->l->ival + 1;
            const int end = get_array_length(type);
            gen_zero_elements(fp, ident, underlying(type), base, start, end);
        }
        break;

    case NOD_LIST:
        gen_init_array(fp, node->l, ident, type);
        gen_init_array(fp, node->r, ident, type);
        break;

    default:
        break;
    }
}

static void gen_init_struct(FILE *fp, const struct ast_node *node,
        const struct ast_node *ident, const struct symbol *struct_sym)
{
    static const struct symbol *sym = NULL;
    static int base = 0;

    sym = struct_sym;

    if (!node)
        return;

    switch (node->kind) {

    case NOD_INIT:
        gen_init_struct(fp, node->l, ident, sym);
        {
            const int offset = base + sym->mem_offset;
            gen_init_scalar(fp, sym->type, ident, offset, node->r);
        }
        sym = sym->next;
        break;

    case NOD_INIT_LIST:
        {
            int tmp;
            base = sym->next->next->mem_offset;

            tmp = base;
            gen_init_struct(fp, node->l, ident, sym->next->next);
            base = tmp;
        }
        {
            /* initialize unspecified elements */
            const struct symbol *start = sym;
            gen_zero_members(fp, ident, base, start);
        }
        break;

    case NOD_LIST:
        gen_init_struct(fp, node->l, ident, sym);
        gen_init_struct(fp, node->r, ident, sym);
        break;

    default:
        break;
    }
}

static void gen_initializer_local(FILE *fp, const struct ast_node *node)
{
    const struct ast_node *ident;

    if (!node || !node->r || node->kind != NOD_DECL_INIT)
        return;

    /* find lvalue ident */
    ident = find_node(node->l, NOD_DECL_IDENT);

    if (ident && is_local_var(ident->sym)) {
        gen_comment(fp, "local var init");

#if 0
        if (is_array(ident->type))
            gen_init_array(fp, node->r, ident, ident->type);
        else
#endif
        if (is_struct(ident->type))
            gen_init_struct(fp, node->r->r /*XXX TMP */, ident, symbol_of(ident->type));
#if 0
        else
            gen_init_scalar(fp, ident->type, ident, 0, node->r->r /*XXX TMP */);
#endif
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
    int size;

    struct symbol *sym;
    struct data_type *type;
    struct ast_node zero_int;
    int base;
    int cur;
};

/*
static void assign_init_struct_(struct memory_byte *bytes,
        const struct data_type *type, const struct ast_node *init)
{
    static struct symbol *member = NULL;

    if (!init)
        return;

    switch (init->kind) {

    case NOD_LIST:
        assign_init_struct_(bytes, type, init->l);
        {
            const int base = 0;
            const int offset = member->mem_offset;
            const int index = base + offset;
            const int size = get_size(member->type);
            int i;

            bytes[index].init = init->r;

            for (i = 0; i < size; i++)
                bytes[index + i].is_written = 1;

        }
        member = member->next;

        break;

    case NOD_INIT_LIST:
        {
            struct symbol *struct_sym = symbol_of(type);
            member = struct_sym->next->next;
        }
        assign_init_struct_(bytes, type, init->l);
        break;

    default:
        break;
    }
}

static int count_element(const struct data_type *type)
{
    if (is_struct(type)) {
        struct symbol *sym = symbol_of(type);
        const int struct_scope = sym->scope_level + 1;
        int i = 0;

        for (; sym; sym = sym->next) {
            if (is_member(sym) && sym->scope_level == struct_scope)
                i += count_element(sym->type);
            if (sym->kind == SYM_SCOPE_END && sym->scope_level == struct_scope)
                break;
        }
        return i;
    }

    if (is_array(type)) {
        const int len = get_array_length(type);
        return len * count_element(underlying(type));
    }

    return 1;
}
*/

static void print_object(struct object_byte *obj)
{
    int i;

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
            const base = elem_size * i;

            zero_clear_bytes(bytes + base, underlying(type));
        }
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
    const size = get_size(ident->type);

    obj->bytes = (struct memory_byte *) calloc(size, sizeof(struct memory_byte));
    obj->size = size;
    obj->sym = ident->sym;
    obj->type = ident->type;

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
            /* get index from node->r */
            const int offset = node->ival;
            const int size = get_size(type);
            const int index = offset * size;

            assign_init(base, type, node->l);
            assign_init(base + index, type, node->r);
        }
        break;

    case NOD_INIT_LIST:
        /* dive into subtype */
        assign_init(base, underlying(type), node->l);
        break;

    case NOD_LIST:
        /* pass to the next initializer */
        assign_init(base, type, node->l);
        assign_init(base, type, node->r);
        break;

    default:
        {
            /* assign initializer to byte */
            const int size = get_size(type);
            int i;

            base->init = node;
            base->type = type;

            for (i = 0; i < size; i++)
                base[i].is_written = 1;
        }
        break;
    }
}

static void gen_object_byte(FILE *fp, const struct object_byte *obj)
{
    int i;

    {
        struct symbol *sym = obj->sym;
        int id = sym->id;

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
            fprintf(fp, "    .zero 1\n");
            continue;
        }

        if (byte->written_size > 0) {
            const int objsize = get_size(byte->type);
            const int val = byte->init ? byte->init->ival : 0;

            switch (objsize) {
            case 1:
                fprintf(fp, "    .byte %d\n", val);
                break;

            case 4:
                fprintf(fp, "    .long %d\n", val);
                break;

            default:
                break;
            }
        }
    }

    fprintf(fp, "\n");
}

static void gen_initializer2(FILE *fp,
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
                gen_init_scalar(fp, byte->type, ident, i, byte->init);
        }
    }

    free_object_byte(&obj);
}

static void gen_initializer_global(FILE *fp, const struct ast_node *node)
{
    const struct ast_node *ident;

    if (!node || node->kind != NOD_DECL_INIT)
        return;

    /* find lvalue ident */
    ident = find_node(node->l, NOD_DECL_IDENT);

    if (ident && is_global_var(ident->sym)) {
        if (is_array(ident->type)) {
#if 0
            if (node->r) {
                gen_init_array(fp, node->r, ident, ident->type);
            } else {
                const int base = 0;
                const int start = 0;
                const int end = get_array_length(ident->type);
                gen_zero_elements(fp, ident, underlying(ident->type), base, start, end);
            }
        fprintf(fp, "\n");
#endif
        }
        /* XXX old initializer doesn't support global struct initialization */
        else if (is_struct(ident->type)) {
            if (node->r) {
                /*
                gen_init_scalar(fp, underlying(ident->type), ident, 0, node->r);
                */
            } else {
                /*
                const int base = 0;
                const int start = 0;
                const int end = get_array_length(ident->type);
                gen_zero_elements(fp, ident, underlying(ident->type), base, start, end);
                */
            }
        }
        else {
#if 0
            struct ast_node *expr = node->r ? node->r->r : NULL;
            gen_init_scalar(fp, ident->type, ident, 0, expr /*XXX TMP */);
#endif
        }
        /*
        fprintf(fp, "\n");
        */
    }
}

static void gen_initializer_global2(FILE *fp, const struct ast_node *node)
{
    const struct ast_node *ident;

    if (!node || node->kind != NOD_DECL_INIT)
        return;

    /* find lvalue ident */
    ident = find_node(node->l, NOD_DECL_IDENT);

    if (ident && is_global_var(ident->sym)) {
        if (is_array(ident->type)) {
            gen_initializer2(fp, ident, node->r);
        }
        else if (is_struct(ident->type)) {
        }
        else {
            gen_initializer2(fp, ident, node->r);
        }
    }
}

static void gen_initializer_local2(FILE *fp, const struct ast_node *node)
{
    const struct ast_node *ident;

    if (!node || !node->r || node->kind != NOD_DECL_INIT)
        return;

    ident = find_node(node->l, NOD_DECL_IDENT);

    if (ident && is_local_var(ident->sym)) {
        if (is_array(ident->type)) {
            gen_initializer2(fp, ident, node->r);
        }
        else if (is_struct(ident->type)) {
        }
        else {
            gen_initializer2(fp, ident, node->r);
        }
    }
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
        gen_ident(fp, node);
        break;

    case NOD_DECL_INIT:
        gen_initializer_local(fp, node);
        gen_initializer_local2(fp, node);
        break;

    case NOD_STRUCT_REF:
        gen_lvalue(fp, node);
        code3__(fp, node, MOV_, addr1(RAX), A_);
        break;

    case NOD_CALL:
        gen_func_call(fp, node);
        break;

    case NOD_FUNC_DEF:
        tmp = scope;
        scope.curr = next_scope++;
        scope.func = scope.curr;

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
        gen_lvalue(fp, node->l);
        code2__(fp, node, PUSH_, RAX);
        gen_code(fp, node->r);
        code2__(fp, node, POP_,  RDX);
        code3__(fp, node, MOV_, A_, addr1(RDX));
        break;

    case NOD_ADD_ASSIGN:
        gen_comment(fp, "add-assign");
        gen_lvalue(fp, node->l);
        code2__(fp, node, PUSH_, RAX);
        gen_code(fp, node->r);
        code2__(fp, node, POP_,  RDX);
        code3__(fp, node, ADD_, A_, addr1(RDX));
        code3__(fp, node, MOV_, addr1(RDX), A_);
        break;

    case NOD_SUB_ASSIGN:
        gen_comment(fp, "sub-assign");
        gen_lvalue(fp, node->l);
        code2__(fp, node, PUSH_, RAX);
        gen_code(fp, node->r);
        code2__(fp, node, POP_,  RDX);
        code3__(fp, node, SUB_, A_, addr1(RDX));
        code3__(fp, node, MOV_, addr1(RDX), A_);
        break;

    case NOD_MUL_ASSIGN:
        gen_comment(fp, "mul-assign");
        gen_lvalue(fp, node->l);
        code2__(fp, node, PUSH_, RAX);
        gen_code(fp, node->r);
        code2__(fp, node, POP_,  RDX);
        code3__(fp, node, IMUL_, addr1(RDX), A_);
        code3__(fp, node, MOV_, A_, addr1(RDX));
        break;

    case NOD_DIV_ASSIGN:
        gen_comment(fp, "div-assign");
        gen_lvalue(fp, node->l);
        code2__(fp, node, PUSH_, RAX);
        gen_code(fp, node->r);
        code3__(fp, node, MOV_, A_, DI_);
        code2__(fp, node, POP_, RSI);
        code3__(fp, node, MOV_, addr1(RSI), A_);
        code1__(fp, node, CLTD_); /* rax -> rdx:rax */
        code2__(fp, node, IDIV_, DI_);
        code3__(fp, node, MOV_, A_, addr1(RSI));
        break;

    case NOD_ADDR:
        gen_lvalue(fp, node->l);
        break;

    case NOD_DEREF:
        gen_code(fp, node->l);
        /* array objects cannot be loaded in registers, and converted to pointers */
        if (!is_array(node->type))
            code3__(fp, node, MOV_, addr1(RAX), A_);
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
        if (is_array(node->l->type)) {
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

    case NOD_NOT:
        gen_code(fp, node->l);
        code3__(fp, node, CMP_, imme(0), A_);
        code2__(fp, node, SETE_,  AL);
        code3__(fp, node, MOVZB_, AL, A_);
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
        gen_initializer_global2(fp, node);
        return;
    }

    gen_global_vars(fp, node->l);
    gen_global_vars(fp, node->r);
}

static void gen_string_literal(FILE *fp, const struct symbol_table *table)
{
    struct symbol *sym;

    for (sym = table->head; sym; sym = sym->next) {
        if (sym->kind == SYM_STRING) {
            fprintf(fp, "_L.str.%d:\n", sym->id);
            fprintf(fp, "    .asciz \"%s\"\n\n", sym->name);
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
