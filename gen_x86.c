#include <stdio.h>
#include <string.h>
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
    /*
    int size;
    */
    const char *suffix;
    const char *directive;
};

const struct data_spec data_spec_table[] = {
    {/*1,*/ "b", "byte  ptr"},
    {/*2,*/ "w", "word  ptr"},
    {/*4,*/ "l", "dword ptr"},
    {/*8,*/ "q", "qword ptr"}
};

/* XXX
static int get_data_size(int tag)
{
    return data_spec_table[tag].size;
}
*/

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

/* XXX disp(base) */
struct operand addr2_pc_rel(struct operand oper, const char *disp)
{
    struct operand o = oper;
    o.kind = OPR_ADDR;
    o.string = disp;

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
                /* XXX */
                fprintf(fp, "_%s(%%%s)", oper->string, reg(oper, tag));
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

static int get_data_tag_from_type2(const struct ast_node *node)
{
    const struct data_type *dt;

    if (node == NULL) {
        return 0;
    }

    dt = node->dtype;

    switch (dt->kind) {
    case DATA_TYPE_CHAR: return BYTE;
    case DATA_TYPE_INT:  return LONG;
    case DATA_TYPE_PTR:  return QUAD;
    default:             return QUAD;
    }
}

static int get_mem_offset(const struct ast_node *node)
{
    return node->data.sym->mem_offset;
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

static void code1__(FILE *fp, const struct ast_node *node,
        struct opecode op)
{
    const struct opecode o0 = op;
    const int tag = get_data_tag_from_type2(node);

    code__(fp, tag, &o0, NULL, NULL);
}

static void code2__(FILE *fp, const struct ast_node *node,
        struct opecode op, struct operand oper1)
{
    const struct opecode o0 = op;
    const struct operand o1 = oper1;
    const int tag = get_data_tag_from_type2(node);

    code__(fp, tag, &o0, &o1, NULL);
}

static void code3__(FILE *fp, const struct ast_node *node,
        struct opecode op, struct operand oper1, struct operand oper2)
{
#if 0
    const struct opecode o0 = op;
    const struct operand o1 = oper1;
    const struct operand o2 = oper2;
    const int tag = get_data_tag_from_type2(node);

    code__(fp, tag, &o0, &o1, &o2);
#endif
    struct opecode o0 = op;
    struct operand o1 = oper1;
    struct operand o2 = oper2;
    int tag = get_data_tag_from_type2(node);

    /* XXX */
    /* this rule comes from x86-64 machine instructions.
     * it depends on the size of register when loading from memory.
     * it is independent of language data types.
     */
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

static int gen_one_param(FILE *fp, const struct ast_node *node)
{
    if (!node) {
        return 0;
    }

    if (node->kind == NOD_PARAM) {
        const int next_index = gen_one_param(fp, node->l);
        const int disp = -get_mem_offset(node);

        code3__(fp, node, MOV_, arg(next_index), addr2(RBP, disp));
        return next_index + 1;
    } else {
        return 0;
    }
}

static void gen_param_list(FILE *fp, const struct ast_node *node)
{
    gen_one_param(fp, node);
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

        /* XXX */
    case NOD_VAR_DEF:
    case NOD_PARAM:
    case NOD_VAR:
        code3__(fp, node, MOV_, BP_, RAX);
        code3__(fp, node, SUB_, imme(get_mem_offset(node)), RAX);
        break;

    case NOD_GLOBAL_VAR:
        code3__(fp, node, LEA_, addr2_pc_rel(RIP, node->data.sym->name), RAX);
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

static void gen_code(FILE *fp, const struct ast_node *node)
{
    static int reg_id = 0;
    static int block_id = 0;

    if (node == NULL) {
        return;
    }

    switch (node->kind) {

    case NOD_GLOBAL:
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
        code3__(fp, node, CMP_, imme(0), A_);
        code2__(fp, node, JE_,  label(block_id, 0));
        /* then */
        gen_code(fp, node->r->l);
        code2__(fp, node, JMP_, label(block_id, 1));
        /* else */
        gen_label(fp, block_id, 0);
        gen_code(fp, node->r->r);
        gen_label(fp, block_id, 1);
        block_id++;
        break;

    case NOD_WHILE:
        gen_label(fp, block_id, 0);
        gen_code(fp, node->l);
        code3__(fp, node, CMP_, imme(0), A_);
        code2__(fp, node, JE_,  label(block_id, 1));
        gen_code(fp, node->r);
        code2__(fp, node, JMP_, label(block_id, 0));
        gen_label(fp, block_id, 1);
        block_id++;
        break;

    case NOD_RETURN:
        gen_code(fp, node->l);
        code3__(fp, node, MOV_, RBP, RSP);
        code2__(fp, node, POP_, RBP);
        code1__(fp, node, RET_);
        break;

    case NOD_PARAM:
    case NOD_VAR:
        {
#if 0
            /* XXX */
            const int disp = -get_mem_offset(node);

            code3__(fp, node, MOV_, addr2(RBP, disp), A_);
#endif
            /* XXX */
            const int disp = -get_mem_offset(node);

            if (node->dtype->kind == DATA_TYPE_ARRAY) {
                code3__(fp, node, LEA_, addr2(RBP, disp), A_);
            } else {
                code3__(fp, node, MOV_, addr2(RBP, disp), A_);
            }
#if 0
            /* XXX */
            const int disp = -get_mem_offset(node);

            switch (node->dtype->kind) {
            case DATA_TYPE_CHAR:
                gen_comment(fp, "==================");
                code3__(fp, node, MOVSB_, addr2(RBP, disp), EAX);
                gen_comment(fp, "==================");
                break;
            case DATA_TYPE_ARRAY:
                code3__(fp, node, LEA_, addr2(RBP, disp), A_);
                break;
            default:
                code3__(fp, node, MOV_, addr2(RBP, disp), A_);
                break;
            }
#endif
        }
        break;

    case NOD_GLOBAL_VAR:
        code3__(fp, node, MOV_, addr2_pc_rel(RIP, node->data.sym->name), RAX);
        break;

    case NOD_VAR_DEF:
        /* XXX */
        gen_lvalue(fp, node);
        code2__(fp, node, PUSH_, RAX);
        gen_code(fp, node->l);
        code2__(fp, node, POP_,  RDX);
        code3__(fp, node, MOV_, A_, addr1(RDX));
        break;

    case NOD_CALL:
        reg_id = 0;
        gen_code(fp, node->l);
        code2__(fp, node, CALL_, str(node->data.sym->name));
        break;

    case NOD_ARG:
        gen_code(fp, node->l);
        gen_code(fp, node->r);
        code3__(fp, node, MOV_, A_, arg(reg_id++));
        break;

    case NOD_FUNC_DEF:
        /* prologue */
        fprintf(fp, "_%s:\n", node->data.sym->name);
        code2__(fp, node, PUSH_, RBP);
        code3__(fp, node, MOV_,  RSP, RBP);
        code3__(fp, node, SUB_, imme(get_mem_offset(node)), RSP);

        /* load params */
        gen_param_list(fp, node->l);
        /* body */
        gen_code(fp, node->r);
        break;

    case NOD_ASSIGN:
        gen_lvalue(fp, node->l);
        code2__(fp, node, PUSH_, RAX);
        gen_code(fp, node->r);
        code2__(fp, node, POP_,  RDX);
        code3__(fp, node, MOV_, A_, addr1(RDX));
        break;

    case NOD_ADDR:
        gen_lvalue(fp, node->l);
        break;

    case NOD_DEREF:
        gen_code(fp, node->l);
        code3__(fp, node, MOV_, addr1(RAX), A_);
#if 0
            /* XXX */
            switch (node->dtype->kind) {
            case DATA_TYPE_CHAR:
                code3__(fp, node, MOVSB_, addr1(RAX), EAX);
                break;
            default:
                code3__(fp, node, MOV_, addr1(RAX), A_);
                break;
            }
#endif
        break;

    case NOD_NUM:
        code3__(fp, node, MOV_, imme(node->data.ival), A_);
        break;

    case NOD_ADD:
        gen_code(fp, node->l);
        code2__(fp, node, PUSH_, RAX);
        gen_code(fp, node->r);
        code2__(fp, node, POP_, RDX);
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

static void gen_global_var_list(FILE *fp, const struct symbol_table *table)
{
    const int N = table->symbol_count;
    int nvars = 0;
    int i;

    /* XXX */
    for (i = 0; i < N; i++) {
        const struct symbol *sym = &table->data[i];

        if (sym->kind == SYM_VAR && sym->scope_level == 0) {
            if (nvars == 0) {
                fprintf(fp, ".data\n");
                fprintf(fp, ".global ");
            }

            if (nvars > 0) {
                fprintf(fp, ", _%s", sym->name);
            } else {
                fprintf(fp, "_%s", sym->name);
            }
            nvars++;
        }
    }

    if (nvars > 0) {
        fprintf(fp, "\n");
    }
}

static void gen_global_var_labels(FILE *fp, const struct symbol_table *table)
{
    const int N = table->symbol_count;
    int nvars = 0;
    int i;

    /* XXX */
    for (i = 0; i < N; i++) {
        const struct symbol *sym = &table->data[i];

        if (sym->kind == SYM_VAR && sym->scope_level == 0) {
            const char *datasize;
            switch (sym->dtype->kind) {
            case DATA_TYPE_CHAR:
                datasize = "byte";
                break;
            case DATA_TYPE_INT:
                datasize = "long";
                break;
            case DATA_TYPE_PTR:
            case DATA_TYPE_ARRAY:
                datasize = "quad";
                break;
            default:
                datasize = "byte";
                break;
            }
            fprintf(fp, "_%s:\n", sym->name);
            /* XXX */
            /*
            fprintf(fp, "    .%s %d\n", datasize, sym->mem_offset);
            */
            fprintf(fp, "    .%s 0\n", datasize);

            nvars++;
        }
    }

    if (nvars > 0) {
        fprintf(fp, "\n");
    }
}

static int gen_global_var_label(FILE *fp, const struct ast_node *node)
{
    int nvars_l = 0;
    int nvars_r = 0;
    int total_nvars = 0;

    if (!node)
        return 0;

    if (node->kind == NOD_GLOBAL) {
        nvars_l = gen_global_var_label(fp, node->l);
        nvars_r = gen_global_var_label(fp, node->r);
        total_nvars = nvars_l + nvars_r;
    }

    if (node->kind == NOD_VAR_DEF) {
        const struct symbol *sym = node->data.sym;

        if (sym->kind == SYM_VAR && sym->scope_level == 0) {
            const char *datasize;
            switch (sym->dtype->kind) {
            case DATA_TYPE_CHAR:
                datasize = "byte";
                break;
            case DATA_TYPE_INT:
                datasize = "long";
                break;
            case DATA_TYPE_PTR:
            case DATA_TYPE_ARRAY:
                datasize = "quad";
                break;
            default:
                datasize = "byte";
                break;
            }
            fprintf(fp, "_%s:\n", sym->name);
            /* XXX */
            if (node->l && node->l->kind == NOD_NUM) {
                fprintf(fp, "    .%s %d\n", datasize, node->l->data.ival);
            } else {
                fprintf(fp, "    .%s 0\n", datasize);
            }

            return total_nvars + 1;
        }
    }
    return total_nvars;
}

static void gen_global_var_label_list(FILE *fp, const struct ast_node *node)
{
    const int nvars = gen_global_var_label(fp, node);
    if (nvars > 0) {
        fprintf(fp, "\n");
    }
}

void gen_x86(FILE *fp,
        const struct ast_node *tree, const struct symbol_table *table)
{
    /* XXX */
    if (!att_syntax) {
        fprintf(fp, ".intel_syntax noprefix\n");
    }

    gen_global_var_list(fp, table);
    /* XXX */
    if (0) {
        gen_global_var_labels(fp, table);
    }
    else
    {
        gen_global_var_label_list(fp, tree);
    }

    fprintf(fp, ".text\n");
    fprintf(fp, ".global ");
    print_global_funcs(fp, tree);
    fprintf(fp, "\n");

    gen_code(fp, tree);
}
