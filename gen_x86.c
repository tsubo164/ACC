#include <stdio.h>
#include "gen_x86.h"

static int att_syntax = 1;

enum data_suffix {
    BYTE = 0,
    WORD,
    LONG,
    QUAD,
    NO_SUFFIX
};

static const int data_size_table[] = {
    1,
    2,
    4,
    8
};

static const char *directive_table[] = {
    "byte  ptr",
    "word  ptr",
    "dword ptr",
    "qword ptr"
};

static const char *AX__[]  = {"al",  "ax", "eax", "rax"};
static const char *BX__[]  = {"bl",  "bx", "ebx", "rbx"};
static const char *CX__[]  = {"cl",  "cx", "ecx", "rcx"};
static const char *DX__[]  = {"dl",  "dx", "edx", "rdx"};
static const char *SI__[]  = {"sil", "si", "esi", "rsi"};
static const char *DI__[]  = {"dil", "di", "edi", "rdi"};
static const char *BP__[]  = {"bpl", "bp", "ebp", "rbp"};
static const char *SP__[]  = {"spl", "sp", "esp", "rsp"};
static const char *R8__[]  = {"r8b", "r8w", "r8d", "r8"};
static const char *R9__[]  = {"r9b", "r9w", "r9d", "r9"};
static const char *MOV_[]  = {"movb",  "movw",  "movl",  "movq",  "mov" };
static const char *ADD_[]  = {"addb",  "addw",  "addl",  "addq",  "add" };
static const char *SUB_[]  = {"subb",  "subw",  "subl",  "subq",  "sub" };
static const char *IMUL_[] = {"imulb", "imulw", "imull", "imulq", "imul" };
static const char *POP_[]  = {"popb",  "popw",  "popl",  "popq",  "pop" };
static const char *PUSH_[] = {"pushb", "pushw", "pushl", "pushq", "push"};
static const char *CALL_[] = {"callb", "callw", "calll", "callq", "call"};
static const char *RET_[]  = {"retb",  "retw",  "retl",  "retq",  "ret" };
static const char **ARG_REG__[] = {DI__, SI__, DX__, CX__, R8__, R9__};

static const char *RAX__[] = {"rax", "rax", "rax", "rax"};
static const char *RDX__[] = {"rdx", "rdx", "rdx", "rdx"};
static const char *RBP__[] = {"rbp", "rbp", "rbp", "rbp"};
static const char *RSP__[] = {"rsp", "rsp", "rsp", "rsp"};

enum operand_kind {
    OPR_NONE,
    OPR_REG,
    OPR_ADDR,
    OPR_IMME,
    OPR_STR
};

struct operand {
    int kind;
    const char **reg_table;
    const char *string;
    long immediate;
    int disp;
};

const struct operand AX_ = {OPR_REG, AX__, 0, 0};
const struct operand BX_ = {OPR_REG, BX__, 0, 0};
const struct operand CX_ = {OPR_REG, CX__, 0, 0};
const struct operand DX_ = {OPR_REG, DX__, 0, 0};
const struct operand SI_ = {OPR_REG, SI__, 0, 0};
const struct operand DI_ = {OPR_REG, DI__, 0, 0};
const struct operand BP_ = {OPR_REG, BP__, 0, 0};
const struct operand SP_ = {OPR_REG, SP__, 0, 0};

const struct operand RAX = {OPR_REG, RAX__, 0, 0};
const struct operand RDX = {OPR_REG, RDX__, 0, 0};
const struct operand RBP = {OPR_REG, RBP__, 0, 0};
const struct operand RSP = {OPR_REG, RSP__, 0, 0};

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

static void code_operand__(FILE *fp, int suffix, const struct operand *oper)
{
    switch (oper->kind) {

    case OPR_NONE:
        break;

    case OPR_REG:
        if (att_syntax) {
            fprintf(fp, "%%%s", oper->reg_table[suffix]);
        } else {
            fprintf(fp, "%s", oper->reg_table[suffix]);
        }
        break;

    case OPR_ADDR:
        if (att_syntax) {
            if (oper->disp != 0) {
                fprintf(fp, "%+d(%%%s)", oper->disp, oper->reg_table[suffix]);
            } else {
                fprintf(fp, "(%%%s)", oper->reg_table[suffix]);
            }
        } else {
            if (oper->disp != 0) {
                fprintf(fp, "%s [%s%+d]",
                    directive_table[suffix], oper->reg_table[suffix], oper->disp);
            } else {
                fprintf(fp, "%s [%s]", directive_table[suffix], oper->reg_table[suffix]);
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

    case OPR_STR:
        fprintf(fp, "_%s", oper->string);
        break;

    default:
        break;
    }
}

static void code__(FILE *fp, int suffix,
        const char **op, const struct operand *oper1, const struct operand *oper2)
{
    const struct operand *o1 = NULL, *o2 = NULL;
    int sfx = NO_SUFFIX;

    fprintf(fp, "    ");

    if (att_syntax) {
        sfx = suffix;
    }

    if (oper1 == NULL && oper2 == NULL) {
        fprintf(fp, "%s",   op[sfx]);
    } else {
        fprintf(fp, "%-5s", op[sfx]);
    }

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
        code_operand__(fp, suffix, o1);
    }

    if (o2) {
        fprintf(fp, ", ");
        code_operand__(fp, suffix, o2);
    }

    fprintf(fp, "\n");
}

static void code1(FILE *fp, int suffix,
        const char **op)
{
    code__(fp, suffix, op, NULL, NULL);
}

static void code2(FILE *fp, int suffix,
        const char **op, struct operand oper1)
{
    struct operand o1 = oper1;

    code__(fp, suffix, op, &o1, NULL);
}

static void code3(FILE *fp, int suffix,
        const char **op, struct operand oper1, struct operand oper2)
{
    struct operand o1 = oper1;
    struct operand o2 = oper2;

    code__(fp, suffix, op, &o1, &o2);
}

static int get_suffix(const struct ast_node *node)
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

static int get_data_size(const struct ast_node *node)
{
    int suffix;

    if (node == NULL) {
        return 0;
    }

    suffix = get_suffix(node);

    return data_size_table[suffix];
}

static int get_offset(const struct ast_node *node)
{
    int size;
    int id;

    if (node == NULL) {
        return 0;
    }

    size = get_data_size(node);
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
        const int offset = get_offset(node);
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

static void gen_params(FILE *fp, const struct ast_node *node)
{
    if (node == NULL) {
        return;
    }

    switch (node->kind) {

    case NOD_PARAM:
        gen_params(fp, node->l);
        {
            const int suffix = get_suffix(node);
            const int index = get_local_var_id(node);
            const int disp = -get_offset(node);

            code3(fp, suffix, MOV_, arg(index), addr2(RBP, disp));
        }
        break;

    default:
        break;
    }
}

static void gen_code(FILE *fp, const struct ast_node *node);

static void gen_comment(FILE *fp, const char *cmt)
{
    fprintf(fp, "## %s\n", cmt);
}

static void gen_lvalue(FILE *fp, const struct ast_node *node)
{
    if (node == NULL) {
        return;
    }

    switch (node->kind) {

    case NOD_PARAM:
    case NOD_VAR:
        /*
        fprintf(fp, "  mov rax, rbp\n");
        fprintf(fp, "  sub rax, %d\n", get_offset(node));
        */
        code3(fp, QUAD, MOV_, BP_, AX_);
        code3(fp, QUAD, SUB_, imme(get_offset(node)), AX_);
        break;

    case NOD_DEREF:
        gen_code(fp, node->l);
        break;

    default:
        gen_comment(fp, "this is not a lvalue");
        break;
    }
}

static void gen_code(FILE *fp, const struct ast_node *node)
{
    static int reg_id = 0;
    static int label_id = 0;

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
        fprintf(fp, "  cmp rax, 0\n");
        fprintf(fp, "  je .L%03d_0\n", label_id);
        /* then */
        gen_code(fp, node->r->l);
        fprintf(fp, "  jmp .L%03d_1\n", label_id);
        /* else */
        fprintf(fp, ".L%03d_0:\n", label_id);
        gen_code(fp, node->r->r);
        fprintf(fp, ".L%03d_1:\n", label_id);
        label_id++;
        break;

    case NOD_WHILE:
        fprintf(fp, ".L%03d_0:\n", label_id);
        gen_code(fp, node->l);
        fprintf(fp, "  cmp rax, 0\n");
        fprintf(fp, "  je .L%03d_1\n", label_id);
        gen_code(fp, node->r);
        fprintf(fp, "  jmp .L%03d_0\n", label_id);
        fprintf(fp, ".L%03d_1:\n", label_id);
        label_id++;
        break;

    case NOD_RETURN:
        gen_code(fp, node->l);
        /* XXX size based on return type */
        code3(fp, QUAD, MOV_, RBP, RSP);
        code2(fp, QUAD, POP_, RBP);
        code1(fp, QUAD, RET_);
        /*
        fprintf(fp, "  mov rsp, rbp\n");
        fprintf(fp, "  pop rbp\n");
        fprintf(fp, "  ret\n");
        */
        break;

    case NOD_PARAM:
    case NOD_VAR:
        {
            const int suffix = get_suffix(node);
            const int disp = -get_offset(node);

            code3(fp, suffix, MOV_, addr2(RBP, disp), AX_);
        }
        break;

    case NOD_VAR_DEF:
        break;

    case NOD_CALL:
        reg_id = 0;
        gen_code(fp, node->l);
        /*
        fprintf(fp, "  call _%s\n", node->data.sym->name);
        */
        code2(fp, QUAD, CALL_, str(node->data.sym->name));
        break;

    case NOD_ARG:
        gen_code(fp, node->l);
        gen_code(fp, node->r);
        /*
        fprintf(fp, "  mov %s, rax\n", argreg[reg_id++]);
        */
        code3(fp, QUAD, MOV_, AX_, arg(reg_id++));
        break;

    case NOD_FUNC_DEF:
        fprintf(fp, "_%s:\n", node->data.sym->name);
        /*
        fprintf(fp, "    push    rbp\n");
        fprintf(fp, "    mov     rbp, rsp\n");
        */
        code2(fp, QUAD, PUSH_, RBP);
        code3(fp, QUAD, MOV_,  RSP, RBP);
        {
            /* XXX tmp */
            int max_offset = get_max_offset(node);
            if (max_offset > 0) {
                if (max_offset % 16 > 0) {
                    max_offset += 16 - max_offset % 16;
                }
                /*
                fprintf(fp, "  sub  rsp, %d\n", max_offset);
                */
                code3(fp, QUAD, SUB_, imme(max_offset), RSP);
            }
        }
        gen_params(fp, node->l);
        gen_code(fp, node->r);
        break;

    case NOD_ASSIGN:
        gen_lvalue(fp, node->l);

        /*
        fprintf(fp, "  push rax\n");
        gen_code(fp, node->r);
        fprintf(fp, "  pop rdx\n");
        fprintf(fp, "  mov  dword ptr[rdx], eax\n");
        */
        /* XXX */
        code2(fp, QUAD, PUSH_, RAX);
        gen_code(fp, node->r);
        code2(fp, QUAD, POP_,  RDX);
        code3(fp, LONG, MOV_, AX_, addr1(RDX));
        break;

    case NOD_ADDR:
        gen_lvalue(fp, node->l);
        break;

    case NOD_DEREF:
        gen_code(fp, node->l);
        fprintf(fp, "  mov rax, [rax]\n");
        break;

    case NOD_NUM:
        /*
        fprintf(fp, "  mov rax, %d\n", node->data.ival);
        */
        code3(fp, QUAD, MOV_, imme(node->data.ival), AX_);
        break;

    case NOD_ADD:
        gen_code(fp, node->l);
        /*
        fprintf(fp, "  push rax\n");
        */
        code2(fp, QUAD, PUSH_, AX_);
        gen_code(fp, node->r);
        /*
        fprintf(fp, "  pop rdx\n");
        fprintf(fp, "  add rax, rdx\n");
        */
        code2(fp, QUAD, POP_, DX_);
        code3(fp, QUAD, ADD_, DX_, AX_);
        break;

    case NOD_SUB:
        gen_code(fp, node->l);
        fprintf(fp, "  push rax\n");
        gen_code(fp, node->r);
        fprintf(fp, "  mov rdx, rax\n");
        fprintf(fp, "  pop rax\n");
        fprintf(fp, "  sub rax, rdx\n");
        break;

    case NOD_MUL:
        gen_code(fp, node->l);
        /*
        fprintf(fp, "  push rax\n");
        */
        code2(fp, QUAD, PUSH_, AX_);
        gen_code(fp, node->r);
        /*
        fprintf(fp, "  pop rdx\n");
        fprintf(fp, "  imul rax, rdx\n");
        */
        code2(fp, QUAD, POP_,  DX_);
        code3(fp, QUAD, IMUL_, DX_, AX_);
        break;

    case NOD_DIV:
        gen_code(fp, node->l);
        fprintf(fp, "  push rax\n");
        gen_code(fp, node->r);
        fprintf(fp, "  mov rdi, rax\n");
        fprintf(fp, "  pop rax\n");
        fprintf(fp, "  cqo\n");
        fprintf(fp, "  idiv rdi\n");
        break;

    case NOD_LT:
        gen_code(fp, node->l);
        fprintf(fp, "  push rax\n");
        gen_code(fp, node->r);
        fprintf(fp, "  mov rdx, rax\n");
        fprintf(fp, "  pop rax\n");
        fprintf(fp, "  cmp rax, rdx\n");
        fprintf(fp, "  setl al\n");
        fprintf(fp, "  movzx rax, al\n");
        break;

    case NOD_GT:
        gen_code(fp, node->l);
        fprintf(fp, "  push rax\n");
        gen_code(fp, node->r);
        fprintf(fp, "  mov rdx, rax\n");
        fprintf(fp, "  pop rax\n");
        fprintf(fp, "  cmp rax, rdx\n");
        fprintf(fp, "  setg al\n");
        fprintf(fp, "  movzx rax, al\n");
        break;

    case NOD_LE:
        gen_code(fp, node->l);
        fprintf(fp, "  push rax\n");
        gen_code(fp, node->r);
        fprintf(fp, "  mov rdx, rax\n");
        fprintf(fp, "  pop rax\n");
        fprintf(fp, "  cmp rax, rdx\n");
        fprintf(fp, "  setle al\n");
        fprintf(fp, "  movzx rax, al\n");
        break;

    case NOD_GE:
        gen_code(fp, node->l);
        fprintf(fp, "  push rax\n");
        gen_code(fp, node->r);
        fprintf(fp, "  mov rdx, rax\n");
        fprintf(fp, "  pop rax\n");
        fprintf(fp, "  cmp rax, rdx\n");
        fprintf(fp, "  setge al\n");
        fprintf(fp, "  movzx rax, al\n");
        break;

    case NOD_EQ:
        gen_code(fp, node->l);
        fprintf(fp, "  push rax\n");
        gen_code(fp, node->r);
        fprintf(fp, "  pop rdx\n");
        fprintf(fp, "  cmp rax, rdx\n");
        fprintf(fp, "  sete al\n");
        fprintf(fp, "  movzx rax, al\n");
        break;

    case NOD_NE:
        gen_code(fp, node->l);
        fprintf(fp, "  push rax\n");
        gen_code(fp, node->r);
        fprintf(fp, "  pop rdx\n");
        fprintf(fp, "  cmp rax, rdx\n");
        fprintf(fp, "  setne al\n");
        fprintf(fp, "  movzx rax, al\n");
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
