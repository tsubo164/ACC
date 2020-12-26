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
    const char *suffix;
    const char *directive;
};

const struct data_spec data_spec_table[] = {
    {"b", "byte  ptr"},
    {"w", "word  ptr"},
    {"l", "dword ptr"},
    {"q", "qword ptr"}
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

static int get_data_tag_from_type2(const struct ast_node *node)
{
    const struct data_type *dt;

    if (node == NULL) {
        return 0;
    }

    dt = node->type;

    switch (dt->kind) {
    case DATA_TYPE_CHAR: return BYTE;
    case DATA_TYPE_INT:  return LONG;
    case DATA_TYPE_PTR:  return QUAD;
    default:             return QUAD;
    }
}

static int get_mem_offset(const struct ast_node *node)
{
    return node->sym->mem_offset;
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
    struct opecode o0 = op;
    struct operand o1 = oper1;
    struct operand o2 = oper2;
    int tag = get_data_tag_from_type2(node);

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

static void gen_func_call(FILE *fp, const struct ast_node *node)
{
    const struct ast_node *ident = NULL;
    ident = find_node(node->l, NOD_IDENT);
    ident = node->l;

    /* args */
    gen_code(fp, node->r);
    /* call */
    code2__(fp, node, CALL_, str(ident->sym->name));
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
        code3__(fp, node, MOV_, addr2_pc_rel(RIP, sym->name), RAX);
    }
    else if (is_enumerator(sym)) {
        code3__(fp, node, MOV_, imme(get_mem_offset(node)), A_);
    }
    else {
        const int disp = -get_mem_offset(node);

        if (node->type->kind == DATA_TYPE_ARRAY) {
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
        code3__(fp, node, LEA_, addr2_pc_rel(RIP, sym->name), RAX);
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

    case NOD_LIST:
        gen_code(fp, node->l);
        gen_code(fp, node->r);
        break;

    case NOD_STMT:
        gen_code(fp, node->l);
        gen_code(fp, node->r);
        break;

    case NOD_COMPOUND:
        gen_code(fp, node->l);
        gen_code(fp, node->r);
        break;

        /* TODO need to walk all nodes by default? */
    case NOD_DECL:
        gen_code(fp, node->l);
        gen_code(fp, node->r);
        break;

        /*
         * for (pre; cond; post)
         *     body;
         * -->
         *     pre
         * label 0
         *     cond ? jne 1
         *     body
         *     post
         *     jmp 0
         * label 1
         */
    case NOD_FOR:
        gen_code(fp, node->l);
        gen_code(fp, node->r);
        block_id++;
        break;

    case NOD_FOR_PRE_COND:
        /* pre */
        gen_comment(fp, "for-pre");
        gen_code(fp, node->l);
        /* cond */
        gen_comment(fp, "for-cond");
        gen_label(fp, block_id, 0);
        gen_code(fp, node->r);
        code3__(fp, node, CMP_, imme(0), A_);
        code2__(fp, node, JE_,  label(block_id, 1));
        break;

    case NOD_FOR_BODY_POST:
        /* body */
        gen_comment(fp, "for-body");
        gen_code(fp, node->l);
        /* post */
        gen_comment(fp, "for-post");
        gen_code(fp, node->r);
        code2__(fp, node, JMP_, label(block_id, 0));
        gen_label(fp, block_id, 1);
        break;

        /*
         * do body;
         * while (cond)
         * -->
         * label 0
         *     body
         *     cond ? jne 1
         *     jmp 0
         * label 1
         */
    case NOD_DOWHILE:
        gen_label(fp, block_id, 0);
        gen_code(fp, node->l);
        gen_code(fp, node->r);
        code3__(fp, node, CMP_, imme(0), A_);
        code2__(fp, node, JE_,  label(block_id, 1));
        code2__(fp, node, JMP_, label(block_id, 0));
        gen_label(fp, block_id, 1);
        block_id++;
        break;

        /*
         * if (cond)
         *     then;
         * else
         *     else;
         * -->
         *     cond ? je 0
         *     then
         *     jmp 1
         * label 0
         *     else
         * label 1
         */
    case NOD_IF:
        /* if */
        gen_code(fp, node->l);
        code3__(fp, node, CMP_, imme(0), A_);
        code2__(fp, node, JE_,  label(block_id, 0));
        gen_code(fp, node->r);
        block_id++;
        break;

    case NOD_IF_THEN:
        gen_comment(fp, "then");
        /* then */
        gen_code(fp, node->l);
        code2__(fp, node, JMP_, label(block_id, 1));
        /* else */
        gen_label(fp, block_id, 0);
        gen_code(fp, node->r);
        gen_label(fp, block_id, 1);
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

    case NOD_IDENT:
        gen_ident(fp, node);
        break;

    case NOD_DECL_INIT:
        {
            const struct ast_node *ident;
            const struct symbol *sym;

            ident = find_node(node->l, NOD_DECL_IDENT);

            /* TODO struct {} may not have ident. IR won't need this if statement */
            if (!ident)
                return;

            sym = ident->sym;

            if (is_local_var(sym)) {
                /* ident */
                gen_lvalue(fp, ident);
                code2__(fp, ident, PUSH_, RAX);

                /* init expr */
                gen_code(fp, node->r);

                /* assign */
                code2__(fp, node, POP_,  RDX);
                code3__(fp, node, MOV_, A_, addr1(RDX));
            }
        }
        break;

    case NOD_STRUCT_REF:
        gen_lvalue(fp, node);
        code3__(fp, node, MOV_, addr1(RAX), A_);
        break;

    case NOD_CALL:
        reg_id = 0;
        gen_func_call(fp, node);
        break;

    case NOD_ARG:
        gen_code(fp, node->l);
        gen_code(fp, node->r);
        code3__(fp, node, MOV_, A_, arg(reg_id++));
        break;

    case NOD_FUNC_DEF:
        gen_func_prologue(fp, node->l);
        gen_func_param_list(fp, node->l);
        gen_func_body(fp, node->r);
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
        break;

    case NOD_NUM:
        code3__(fp, node, MOV_, imme(node->ival), A_);
        break;

    case NOD_STRING:
        code3__(fp, node, LEA_, label__("L.str", node->ival), A_);
        break;

    case NOD_ADD:
        gen_code(fp, node->l);
        code2__(fp, node, PUSH_, RAX);
        gen_code(fp, node->r);
        code2__(fp, node, POP_, RDX);

        /* TODO find the best place to handle array subscript */
        if (node->l->type->kind == DATA_TYPE_ARRAY) {
            const int sz = node->l->type->ptr_to->byte_size;
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

    case NOD_PREINC:
        gen_lvalue(fp, node->l);
        code3__(fp, node, ADD_, imme(1), addr1(RAX));
        code3__(fp, node, MOV_, addr1(RAX), A_);
        break;

    case NOD_PREDEC:
        gen_lvalue(fp, node->l);
        code3__(fp, node, SUB_, imme(1), addr1(RAX));
        code3__(fp, node, MOV_, addr1(RAX), A_);
        break;

    case NOD_POSTINC:
        gen_lvalue(fp, node->l);
        code3__(fp, node, MOV_, RAX, RDX);
        code3__(fp, node, MOV_, addr1(RAX), A_);
        code3__(fp, node, ADD_, imme(1), addr1(RDX));
        break;

    case NOD_POSTDEC:
        gen_lvalue(fp, node->l);
        code3__(fp, node, MOV_, RAX, RDX);
        code3__(fp, node, MOV_, addr1(RAX), A_);
        code3__(fp, node, SUB_, imme(1), addr1(RDX));
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

static void gen_global_func_list(FILE *fp, const struct symbol_table *table)
{
    const int N = get_symbol_count(table);
    int i;
    int nfuncs = 0;

    for (i = 0; i < N; i++) {
        const struct symbol *sym = &table->data[i];

        if (sym->kind == SYM_FUNC) {
            if (nfuncs > 0)
                fprintf(fp, ", ");

            fprintf(fp, "_%s", sym->name);
            nfuncs++;
        }
    }

    fprintf(fp, "\n");
}

static void gen_global_var_list(FILE *fp, const struct symbol_table *table)
{
    const int N = get_symbol_count(table);
    int nvars = 0;
    int i;

    for (i = 0; i < N; i++) {
        const struct symbol *sym = &table->data[i];

        if (is_global_var(sym)) {
            if (nvars == 0) {
                fprintf(fp, ".data\n");
                fprintf(fp, ".global ");

                fprintf(fp, "_%s", sym->name);
            } else {
                fprintf(fp, ", _%s", sym->name);
            }

            nvars++;
        }
    }

    if (nvars > 0)
        fprintf(fp, "\n");
}

static void gen_global_var_labels(FILE *fp, const struct ast_node *node)
{
    if (!node)
        return;

    if (node->kind == NOD_DECL_INIT) {
        const struct ast_node *ident;
        const struct symbol *sym;

        ident = find_node(node, NOD_DECL_IDENT);

        /* TODO struct {} may not have ident. IR won't need this if statement */
        if (!ident)
            return;

        sym = ident->sym;

        if (is_global_var(sym)) {
            /* TODO need eval instead of find NOD_NUM */
            const struct ast_node *init = find_node(node, NOD_NUM);
            const int val = init ? init->ival : 0;

            const char *datasize;
            switch (sym->type->kind) {
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
            fprintf(fp, "    .%s %d\n", datasize, val);
        }

        return;
    }

    gen_global_var_labels(fp, node->l);
    gen_global_var_labels(fp, node->r);
}

static void gen_string_literal(FILE *fp, const struct ast_node *node)
{
    static int n = 0;

    if (!node)
        return;

    if (node->kind == NOD_STRING) {
        if (n == 0)
            fprintf(fp, ".data\n");
        if (node->ival == 0)
            fprintf(fp, "_L.str:\n");
        else
            fprintf(fp, "_L.str.%d:\n", node->ival);
        fprintf(fp, "    .asciz \"%s\"\n", node->sval);
        n++;
        return;
    }

    gen_string_literal(fp, node->l);
    gen_string_literal(fp, node->r);
}

void gen_x86(FILE *fp,
        const struct ast_node *tree, const struct symbol_table *table)
{
    if (!att_syntax) {
        fprintf(fp, ".intel_syntax noprefix\n");
    }

    gen_string_literal(fp, tree);
    gen_global_var_list(fp, table);
    gen_global_var_labels(fp, tree);

    fprintf(fp, ".text\n");
    fprintf(fp, ".global ");

    gen_global_func_list(fp, table);

    gen_code(fp, tree);
}
