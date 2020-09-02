#include <stdio.h>
#include "gen_x86.h"

static int att_syntax = 1;

enum data_tag {
    BYTE = 0,
    WORD,
    LONG,
    QUAD,
    NO_SUFFIX
};

struct data_spec {
    int size;
    const char *suffix;
    const char *directive;
};

const struct data_spec data_spec_table[] = {
    {1, "b", "byte  ptr"},
    {2, "w", "word  ptr"},
    {4, "l", "dword ptr"},
    {8, "q", "qword ptr"}
};

static int get_data_size(int tag)
{
    return data_spec_table[tag].size;
}

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
static const char *SI__[]  = {"sil", "si", "esi", "rsi"};
static const char *DI__[]  = {"dil", "di", "edi", "rdi"};
static const char *BP__[]  = {"bpl", "bp", "ebp", "rbp"};
static const char *SP__[]  = {"spl", "sp", "esp", "rsp"};
static const char *R8__[]  = {"r8b", "r8w", "r8d", "r8"};
static const char *R9__[]  = {"r9b", "r9w", "r9d", "r9"};
static const char **ARG_REG__[] = {DI__, SI__, D__, C__, R8__, R9__};

static const char *AL__[]   = {"al",  "al", "al", "al"};
static const char *RAX__[]  = {"rax",  "rax", "rax", "rax"};
static const char *RDX__[]  = {"rdx",  "rdx", "rdx", "rdx"};
static const char *RBP__[]  = {"rbp",  "rbp", "rbp", "rbp"};
static const char *RSP__[]  = {"rsp",  "rsp", "rsp", "rsp"};

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
const struct opecode POP_   = {"pop",   1};
const struct opecode PUSH_  = {"push",  1};
const struct opecode CALL_  = {"call",  1};
const struct opecode RET_   = {"ret",   1};
const struct opecode MOVZB_ = {"movzb", 1};

const struct opecode JE_    = {"je",  0};
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
    OPR_STR
};

struct operand {
    int kind;
    const char **reg_table;
    const char *string;
    long immediate;
    int disp;
    int label_id;
    int block_id;
};

/* dynamic name registers */
const struct operand A_  = {OPR_REG, A__,  NULL, 0, 0};
const struct operand B_  = {OPR_REG, B__,  NULL, 0, 0};
const struct operand C_  = {OPR_REG, C__,  NULL, 0, 0};
const struct operand D_  = {OPR_REG, D__,  NULL, 0, 0};
const struct operand SI_ = {OPR_REG, SI__, NULL, 0, 0};
const struct operand DI_ = {OPR_REG, DI__, NULL, 0, 0};
const struct operand BP_ = {OPR_REG, BP__, NULL, 0, 0};
const struct operand SP_ = {OPR_REG, SP__, NULL, 0, 0};

/* static name registers */
const struct operand AL  = {OPR_REG, AL__, NULL, 0, 0};

const struct operand RAX = {OPR_REG, RAX__, NULL, 0, 0};
const struct operand RDX = {OPR_REG, RDX__, NULL, 0, 0};
const struct operand RBP = {OPR_REG, RBP__, NULL, 0, 0};
const struct operand RSP = {OPR_REG, RSP__, NULL, 0, 0};

/* 2, 0x8, ... */
struct operand imme(long value)
{
    struct operand o = {0};
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

/* _main, .L001, ... */
struct operand str(const char *value)
{
    struct operand o = {0};
    o.kind = OPR_STR;
    o.string = value;

    return o;
}

/* rdi, rsi, ... */
struct operand arg(int index)
{
    struct operand o = {0};
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

static const char *reg(const struct operand *oper, int tag)
{
    return oper->reg_table[tag];
}

static void gen_opecode__(FILE *fp, int tag, const struct opecode *op)
{
    int n = 0;
    int pad = 0;
    const char *sfx = "";

    if (att_syntax) {
        sfx = get_data_suffix(tag);
    }

    if (!op->has_suffix) {
        sfx = "";
    }

    fprintf(fp, "%s%s%n", op->mnemonic, sfx, &n);

    pad = n > 6 ? 0 : 6 - n;

    fprintf(fp, "%*s", pad, "");
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

    fprintf(fp, "    ");

    gen_opecode__(fp, tag, op);

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
        fprintf(fp, " ");
        gen_operand__(fp, tag, o1);
    }

    if (o2) {
        fprintf(fp, ", ");
        gen_operand__(fp, tag, o2);
    }

    fprintf(fp, "\n");
}

static void code1(FILE *fp, int tag,
        struct opecode op)
{
    struct opecode o0 = op;
    code__(fp, tag, &o0, NULL, NULL);
}

static void code2(FILE *fp, int tag,
        struct opecode op, struct operand oper1)
{
    struct opecode o0 = op;
    struct operand o1 = oper1;

    code__(fp, tag, &o0, &o1, NULL);
}

static void code3(FILE *fp, int tag,
        struct opecode op, struct operand oper1, struct operand oper2)
{
    struct opecode o0 = op;
    struct operand o1 = oper1;
    struct operand o2 = oper2;

    code__(fp, tag, &o0, &o1, &o2);
}

static int get_data_tag_from_type(const struct ast_node *node)
{
    int type;

    if (node == NULL) {
        return 0;
    }

    type = node->data.sym->data_type;

    switch (type) {
    case TYP_INT: return LONG;
    default:      return QUAD;
    }
}

static int get_local_var_id(const struct ast_node *node)
{
    if (node == NULL) {
        return 0;
    }

    return node->data.sym->local_var_id;
}

static int get_mem_offset(const struct ast_node *node)
{
    int size;
    int id;

    if (node == NULL) {
        return 0;
    }

    size = get_data_size(get_data_tag_from_type(node));
    id = node->data.sym->local_var_id;

    /* id starts with 0 */
    return size * (id + 1);
}

static int get_max_offset(const struct ast_node *node)
{
    int max, l, r;

    if (node == NULL) {
        return 0;
    }

    l = get_max_offset(node->l);
    r = get_max_offset(node->r);
    max = l > r ? l : r;

    if (node->kind == NOD_VAR || node->kind == NOD_PARAM) {
        const int offset = get_mem_offset(node);
        return offset > max ? offset : max;
    } else {
        return max;
    }
}

static void print_global_funcs(FILE *fp, const struct ast_node *node)
{
    static int nfuncs = 0;

    if (node == NULL) {
        return;
    }

    print_global_funcs(fp, node->l);
    print_global_funcs(fp, node->r);

    if (node->kind == NOD_FUNC_DEF) {
        if (nfuncs > 0) {
            fprintf(fp, ", ");
        }
        fprintf(fp, "_%s", node->data.sym->name);
        nfuncs++;
    }
}

/* forward declaration */
static void gen_code(FILE *fp, const struct ast_node *node);

static void gen_params(FILE *fp, const struct ast_node *node)
{
    if (node == NULL) {
        return;
    }

    switch (node->kind) {

    case NOD_PARAM:
        gen_params(fp, node->l);
        {
            const int tag = get_data_tag_from_type(node);
            const int index = get_local_var_id(node);
            const int disp = -get_mem_offset(node);

            code3(fp, tag, MOV_, arg(index), addr2(RBP, disp));
        }
        break;

    default:
        break;
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

static void gen_lvalue(FILE *fp, const struct ast_node *node)
{
    if (node == NULL) {
        return;
    }

    switch (node->kind) {

    case NOD_PARAM:
    case NOD_VAR:
        code3(fp, QUAD, MOV_, BP_, A_);
        code3(fp, QUAD, SUB_, imme(get_mem_offset(node)), A_);
        break;

    case NOD_DEREF:
        gen_code(fp, node->l);
        break;

    default:
        gen_comment(fp, "this is not a lvalue");
        break;
    }
}

static void gen_relational(FILE *fp, const struct ast_node *node, struct opecode op)
{
    gen_code(fp, node->l);
    code2(fp, QUAD, PUSH_, A_);
    gen_code(fp, node->r);
    code3(fp, QUAD, MOV_, A_, D_);
    code2(fp, QUAD, POP_, A_);
    code3(fp, QUAD, CMP_, D_, A_);
    code2(fp, QUAD, op, AL);
    code3(fp, QUAD, MOVZB_, AL, A_);
}

static void gen_equality(FILE *fp, const struct ast_node *node, struct opecode op)
{
    gen_code(fp, node->l);
    code2(fp, QUAD, PUSH_, A_);
    gen_code(fp, node->r);
    code2(fp, QUAD, POP_, D_);
    code3(fp, QUAD, CMP_, D_, A_);
    code2(fp, QUAD, op,   AL);
    code3(fp, QUAD, MOVZB_, AL, A_);
}

static void gen_code(FILE *fp, const struct ast_node *node)
{
    static int reg_id = 0;
    static int block_id = 0;

    if (node == NULL) {
        return;
    }

    switch (node->kind) {

    case NOD_LIST:
        gen_code(fp, node->l);
        gen_code(fp, node->r);
        break;

    case NOD_STMT:
        gen_code(fp, node->l);
        gen_code(fp, node->r);
        break;

    case NOD_IF:
        /* if */
        gen_code(fp, node->l);
        code3(fp, QUAD, CMP_, imme(0), A_);
        code2(fp, QUAD, JE_,  label(block_id, 0));
        /* then */
        gen_code(fp, node->r->l);
        code2(fp, QUAD, JMP_, label(block_id, 1));
        /* else */
        gen_label(fp, block_id, 0);
        gen_code(fp, node->r->r);
        gen_label(fp, block_id, 1);
        block_id++;
        break;

    case NOD_WHILE:
        gen_label(fp, block_id, 0);
        gen_code(fp, node->l);
        code3(fp, QUAD, CMP_, imme(0), A_);
        code2(fp, QUAD, JE_,  label(block_id, 1));
        gen_code(fp, node->r);
        code2(fp, QUAD, JMP_, label(block_id, 0));
        gen_label(fp, block_id, 1);
        block_id++;
        break;

    case NOD_RETURN:
        gen_code(fp, node->l);
        /* XXX type */
        code3(fp, QUAD, MOV_, RBP, RSP);
        code2(fp, QUAD, POP_, RBP);
        code1(fp, QUAD, RET_);
        break;

    case NOD_PARAM:
    case NOD_VAR:
        {
            const int tag = get_data_tag_from_type(node);
            const int disp = -get_mem_offset(node);

            code3(fp, tag, MOV_, addr2(RBP, disp), A_);
        }
        break;

    case NOD_VAR_DEF:
        break;

    case NOD_CALL:
        reg_id = 0;
        gen_code(fp, node->l);
        code2(fp, QUAD, CALL_, str(node->data.sym->name));
        break;

    case NOD_ARG:
        gen_code(fp, node->l);
        gen_code(fp, node->r);
        code3(fp, QUAD, MOV_, A_, arg(reg_id++));
        break;

    case NOD_FUNC_DEF:
        fprintf(fp, "_%s:\n", node->data.sym->name);
        code2(fp, QUAD, PUSH_, RBP);
        code3(fp, QUAD, MOV_,  RSP, RBP);
        {
            /* XXX tmp */
            int max_offset = get_max_offset(node);
            if (max_offset > 0) {
                if (max_offset % 16 > 0) {
                    max_offset += 16 - max_offset % 16;
                }
                code3(fp, QUAD, SUB_, imme(max_offset), RSP);
            }
        }
        gen_params(fp, node->l);
        gen_code(fp, node->r);
        break;

    case NOD_ASSIGN:
        gen_lvalue(fp, node->l);
        /* XXX type */
        code2(fp, QUAD, PUSH_, RAX);
        gen_code(fp, node->r);
        code2(fp, QUAD, POP_,  RDX);
        code3(fp, LONG, MOV_, A_, addr1(RDX));
        break;

    case NOD_ADDR:
        gen_lvalue(fp, node->l);
        break;

    case NOD_DEREF:
        gen_code(fp, node->l);
        fprintf(fp, "  mov rax, [rax]\n");
        break;

    case NOD_NUM:
        code3(fp, QUAD, MOV_, imme(node->data.ival), A_);
        break;

    case NOD_ADD:
        gen_code(fp, node->l);
        code2(fp, QUAD, PUSH_, A_);
        gen_code(fp, node->r);
        code2(fp, QUAD, POP_, D_);
        code3(fp, QUAD, ADD_, D_, A_);
        break;

    case NOD_SUB:
        gen_code(fp, node->l);
        code2(fp, QUAD, PUSH_, A_);
        gen_code(fp, node->r);
        code3(fp, QUAD, MOV_, A_, D_);
        code2(fp, QUAD, POP_, A_);
        code3(fp, QUAD, SUB_, D_, A_);
        break;

    case NOD_MUL:
        gen_code(fp, node->l);
        code2(fp, QUAD, PUSH_, A_);
        gen_code(fp, node->r);
        code2(fp, QUAD, POP_,  D_);
        code3(fp, QUAD, IMUL_, D_, A_);
        break;

    case NOD_DIV:
        gen_code(fp, node->l);
        code2(fp, QUAD, PUSH_, A_);
        gen_code(fp, node->r);
        code3(fp, QUAD, MOV_, A_, DI_);
        code2(fp, QUAD, POP_, A_);
        code1(fp, QUAD, CLTD_); /* rax -> rdx:rax */
        code2(fp, QUAD, IDIV_, DI_);
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

void gen_x86(FILE *fp, const struct ast_node *tree)
{
    if (!att_syntax) {
        fprintf(fp, ".intel_syntax noprefix\n");
    }
    fprintf(fp, ".global ");
    print_global_funcs(fp, tree);
    fprintf(fp, "\n");

    gen_code(fp, tree);
}