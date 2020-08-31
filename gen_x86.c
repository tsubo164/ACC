#include <stdio.h>
#include "gen_x86.h"

/* x86-64 calling convention */
/*
static const char *argreg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
*/

enum register_name {
    AX = 0,
    BX,
    CX,
    DX,
    SI,
    DI,
    BP,
    SP
};

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

static const char *arg_register_table[][6] = {
    {"dil", "sil", "dl",  "cl",  "r8b", "r9b"},
    {"di",  "si",  "dx",  "cx",  "r8w", "r9w"},
    {"edi", "esi", "edx", "ecx", "r8d", "r9d"},
    {"rdi", "rsi", "rdx", "rcx", "r8",  "r9" }
};

static const char *gen_register_table[][8] = {
    {"al",  "bl",  "cl",  "dl",  "sil", "dil", "bpl", "spl"},
    {"ax",  "bx",  "cx",  "dx",  "si",  "di" , "bp",  "sp" },
    {"eax", "ebx", "ecx", "edx", "esi", "edi", "ebp", "esp"},
    {"rax", "rbx", "rcx", "rdx", "rsi", "rdi", "rbp", "rsp"}
};

/*
static const char *ARG_REG_[][6] = {
    {"dil", "di",  "edi", "rdi"},
    {"sil", "si",  "esi", "rsi"},
    {"dl",  "dx",  "edx", "rdx"},
    {"cl",  "cx",  "ecx", "rcx"},
    {"r8b", "r8w", "r8d", "r8"},
    {"r9b", "r9w", "r9d", "r9"}
};
*/

static const char *gen_register_table_[][4] = {
    {"al",  "ax", "eax", "rax"},
    {"bl",  "bx", "ebx", "rbx"},
    {"cl",  "cx", "ecx", "rcx"},
    {"dl",  "dx", "edx", "rdx"},
    {"sil", "si", "esi", "rsi"},
    {"dil", "di", "edi", "rdi"},
    {"bpl", "bp", "ebp", "rbp"},
    {"spl", "sp", "esp", "rsp"}
};

enum data_suffix_ { B, W, L, Q};

/*
static const char *ax(int suffix)
{
    static const char *r[] = {"al", "ax", "eax", "rax"};
    return r[suffix];
}
*/

enum opcode {
    mov, add, sub, pop, push, ret
};

const char *AX_[] = {"al",  "ax", "eax", "rax"};
const char *BX_[] = {"bl",  "bx", "ebx", "rbx"};
const char *CX_[] = {"cl",  "cx", "ecx", "rcx"};
const char *DX_[] = {"dl",  "dx", "edx", "rdx"};
const char *SI_[] = {"sil", "si", "esi", "rsi"};
const char *DI_[] = {"dil", "di", "edi", "rdi"};
const char *BP_[] = {"bpl", "bp", "ebp", "rbp"};
const char *SP_[] = {"spl", "sp", "esp", "rsp"};
const char *R8_[] = {"r8b", "r8w", "r8d", "r8"};
const char *R9_[] = {"r9b", "r9w", "r9d", "r9"};
const char *MOV_[]  = {"movb",  "movw",  "movl",  "movq",  "mov" };
const char *ADD_[]  = {"addb",  "addw",  "addl",  "addq",  "add" };
const char *SUB_[]  = {"subb",  "subw",  "subl",  "subq",  "sub" };
const char *POP_[]  = {"popb",  "popw",  "popl",  "popq",  "pop" };
const char *PUSH_[] = {"pushb", "pushw", "pushl", "pushq", "push"};
const char *RET_[]  = {"retb",  "retw",  "retl",  "retq",  "ret" };
const char **ARG_REG_[] = {DI_, SI_, DX_, CX_, R8_, R9_};

/*
static const char *opcode_table[][6] = {
    {"movb", "addb", "subb", "popb", "pushb", "retb"},
    {"movw", "addw", "subw", "popw", "pushw", "retw"},
    {"movl", "addl", "subl", "popl", "pushl", "retl"},
    {"movq", "addq", "subq", "popq", "pushq", "retq"},
};
*/
static const char *opcode_table[][5] = {
    {"movb",  "movw",  "movl",  "movq",  "mov" },
    {"addb",  "addw",  "addl",  "addq",  "add" },
    {"subb",  "subw",  "subl",  "subq",  "sub" },
    {"popb",  "popw",  "popl",  "popq",  "pop" },
    {"pushb", "pushw", "pushl", "pushq", "push"},
    {"retb",  "retw",  "retl",  "retq",  "ret" }
};

/*
static const char *opcode_suffix_table[] = {
    "b",
    "w",
    "l",
    "q"
};
*/
const char *PUSH__ = "    push%s    %s\n";

const char *MOV  = "mov";
const char *ADD  = "add";
const char *SUB  = "sub";
const char *POP  = "pop";
const char *PUSH = "push";
const char *RET  = "ret";

static void emit1(FILE *fp, const char *op, int suffix)
{
    fprintf(fp, "    %s\n", op);
}

static void emit2(FILE *fp, const char *op, int suffix, int dst)
{
    const char *d = gen_register_table[suffix][dst];

    fprintf(fp, "    %-4s    %s\n", op, d);
}

static void emit3(FILE *fp, const char *op, int suffix, int src, int dst)
{
    const char *s = gen_register_table[suffix][src];
    const char *d = gen_register_table[suffix][dst];

    fprintf(fp, "    %-4s    %s, %s\n", op, d, s);
}

static void opimm(FILE *fp, const char *op, int suffix, long imm, int dst)
{
    const char *d = gen_register_table[suffix][dst];

    fprintf(fp, "    %-4s    %s, %ld\n", op, d, imm);
}

static void emit___(FILE *fp, const char *op, const char *opr1, const char *opr2)
{
    if (opr1 == NULL || opr2 == NULL) {
        fprintf(fp, "    %s", op);
    } else {
        fprintf(fp, "    %-4s", op);
    }

    if (opr1 != NULL) {
        fprintf(fp, "    %s", opr1);
    }

    if (opr2 != NULL) {
        fprintf(fp, ", %s", opr2);
    }

    fprintf(fp, "\n");
}

/*
static void emit3_(FILE *fp, int suffix, const char **op, const char **src, const char **dst)
{
    emit___(fp, op[suffix], src[suffix], dst[suffix]);
}
*/

static void mov_(FILE *fp, int suffix, const char **src, const char **dst)
{
    emit___(fp, opcode_table[mov][NO_SUFFIX], src[suffix], dst[suffix]);
    /*
    emit___(fp, MOV_[NO_SUFFIX], src[suffix], dst[suffix]);
    */
}

static void call_(FILE *fp, const char *name)
{
    char buf[256] = {'\0'};
    sprintf(buf, "_%s", name);
    emit___(fp, "call", buf, NULL);
    /*
    fprintf(fp, "    call    _%s\n", name);
    */
}

static void code(FILE *fp, int suffix, const char **op, const char **src, const char **dst)
{
    const char *o = op[NO_SUFFIX];
    const char *s = src ? src[suffix] : NULL;
    const char *d = dst ? dst[suffix] : NULL;

    emit___(fp, o, s, d);
}

/*
static void call_(FILE *fp, const char *name)
{
    fprintf(fp, "    call    _%s\n", name);
}

*/

#if 0
static void mov_(FILE *fp, int suffix, int src, int dst)
{
    const char *s = gen_register_table[suffix][src];
    const char *d = gen_register_table[suffix][dst];

    fprintf(fp, "    mov     %s, %s\n", d, s);
}

static void pop_(FILE *fp, int suffix, int dst)
{
    const char *d = gen_register_table[suffix][dst];

    fprintf(fp, "    pop     %s\n", d);
}

static void ret_(FILE *fp, int suffix)
{
    fprintf(fp, "    ret\n");
}
#endif
/*
*/

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

#if n
static void store(FILE *file, int suffix, int base, int disp, int src_reg)
{
    int suffix;

    if (node == NULL) {
        return;
    }

    suffix = get_suffix(node);

    if (disp > 0) {
        fprintf(file, "    mov     %s [%s-%d], %s\n",
                directive_table[suffix],
                gen_register_table[QUAD][base],
                disp,
                gen_register_table[suffix][src_reg]);
    } else {
        fprintf(file, "    mov     %s [%s], %s\n",
                directive_table[suffix],
                gen_register_table[QUAD][base],
                gen_register_table[suffix][src_reg]);
    }
}
#endif

static void load_to(FILE *file, const struct ast_node *node, int reg_name)
{
    int suffix;

    if (node == NULL) {
        return;
    }

    suffix = get_suffix(node);

    fprintf(file, "    mov     %s, %s [rbp-%d]\n",
            gen_register_table[suffix][reg_name],
            directive_table[suffix],
            get_offset(node));
}

static void store_param(FILE *file, const struct ast_node *node)
{
    int suffix;
    int index;

    if (node == NULL) {
        return;
    }

    suffix = get_suffix(node);
    index = get_local_var_id(node);

    fprintf(file, "    mov     %s [rbp-%d], %s\n",
            directive_table[suffix],
            get_offset(node),
            arg_register_table[suffix][index]);
}

static void gen_params(FILE *file, const struct ast_node *node)
{
    if (node == NULL) {
        return;
    }

    switch (node->kind) {

    case NOD_PARAM:
        gen_params(file, node->l);
        store_param(file, node);
        break;

    default:
        break;
    }
}

static void gen_code(FILE *file, const struct ast_node *node);

static void gen_comment(FILE *file, const char *cmt)
{
    fprintf(file, "## %s\n", cmt);
}

static void gen_lvalue(FILE *file, const struct ast_node *node)
{
    if (node == NULL) {
        return;
    }

    switch (node->kind) {

    case NOD_PARAM:
    case NOD_VAR:
        /*
        fprintf(file, "  mov rax, rbp\n");
        fprintf(file, "  sub rax, %d\n", get_offset(node));
        */
        emit3(file, MOV, QUAD, BP, AX);
        opimm(file, SUB, QUAD, get_offset(node), AX);
        break;

    case NOD_DEREF:
        gen_code(file, node->l);
        break;

    default:
        gen_comment(file, "this is not a lvalue");
        break;
    }
}

static void gen_code(FILE *file, const struct ast_node *node)
{
    static int reg_id = 0;
    static int label_id = 0;

    if (node == NULL) {
        return;
    }

    switch (node->kind) {

    case NOD_LIST:
        gen_code(file, node->l);
        gen_code(file, node->r);
        break;

    case NOD_STMT:
        gen_code(file, node->l);
        gen_code(file, node->r);
        break;

    case NOD_IF:
        /* if */
        gen_code(file, node->l);
        fprintf(file, "  cmp rax, 0\n");
        fprintf(file, "  je .L%03d_0\n", label_id);
        /* then */
        gen_code(file, node->r->l);
        fprintf(file, "  jmp .L%03d_1\n", label_id);
        /* else */
        fprintf(file, ".L%03d_0:\n", label_id);
        gen_code(file, node->r->r);
        fprintf(file, ".L%03d_1:\n", label_id);
        label_id++;
        break;

    case NOD_WHILE:
        fprintf(file, ".L%03d_0:\n", label_id);
        gen_code(file, node->l);
        fprintf(file, "  cmp rax, 0\n");
        fprintf(file, "  je .L%03d_1\n", label_id);
        gen_code(file, node->r);
        fprintf(file, "  jmp .L%03d_0\n", label_id);
        fprintf(file, ".L%03d_1:\n", label_id);
        label_id++;
        break;

    case NOD_RETURN:
        gen_code(file, node->l);
        /* XXX size based on return type */
        emit3(file, MOV, QUAD, BP, SP);
        emit2(file, POP, QUAD, BP);
        emit1(file, RET, QUAD);
#if 0
        mov_(file, QUAD, BP, SP);
        pop_(file, QUAD, BP);
        ret_(file, QUAD);
#endif
        /*
        fprintf(file, "  mov rsp, rbp\n");
        fprintf(file, "  pop rbp\n");
        fprintf(file, "  ret\n");
        */
        break;

    case NOD_PARAM:
    case NOD_VAR:
        load_to(file, node, AX);
        break;

    case NOD_VAR_DEF:
        break;

    case NOD_CALL:
        reg_id = 0;
        gen_code(file, node->l);
        /*
        fprintf(file, "  call _%s\n", node->data.sym->name);
        */
        call_(file, node->data.sym->name);
        break;

    case NOD_ARG:
        gen_code(file, node->l);
        gen_code(file, node->r);
        /*
        fprintf(file, "  mov %s, rax\n", argreg[reg_id++]);
        */
#if 0
        emit___(file, "mov", arg_register_table[L][reg_id++], ax(L));
#endif
#if 0
        mov_(file, QUAD, ARG_REG_[reg_id++], gen_register_table_[AX]);
#endif
        code(file, QUAD, MOV_, ARG_REG_[reg_id++], AX_);
        break;

    case NOD_FUNC_DEF:
        fprintf(file, "_%s:\n", node->data.sym->name);
#if 0
        fprintf(file, PUSH__, "", "rbp");
#endif
        fprintf(file, "    push    rbp\n");
        fprintf(file, "    mov     rbp, rsp\n");
        {
            /* XXX tmp */
            int max_offset = get_max_offset(node);
            if (max_offset > 0) {
                if (max_offset % 16 > 0) {
                    max_offset += 16 - max_offset % 16;
                }
                fprintf(file, "    sub     rsp, %d\n", max_offset);
            }
        }
        gen_params(file, node->l);
        gen_code(file, node->r);
        break;

    case NOD_ASSIGN:
        gen_lvalue(file, node->l);

        emit2(file, PUSH, QUAD, AX);
        gen_code(file, node->r);
        emit2(file, POP,  QUAD, DX);
/*
        fprintf(file, "  push rax\n");
        gen_code(file, node->r);
        fprintf(file, "  pop rdx\n");
*/
#if 0
        fprintf(file, "  mov [rdx], rax\n");

        fprintf(file, "  mov rax, [rdx]\n");
        store_from(file, node->l, DX);
#endif
        /* XXX */
        fprintf(file, "    mov     dword ptr[rdx], eax\n");
        break;

    case NOD_ADDR:
        gen_lvalue(file, node->l);
        break;

    case NOD_DEREF:
        gen_code(file, node->l);
        fprintf(file, "  mov rax, [rax]\n");
        break;

    case NOD_NUM:
        /*
        fprintf(file, "  mov rax, %d\n", node->data.ival);
        */
        opimm(file, MOV, QUAD, node->data.ival, AX);
        break;

    case NOD_ADD:
        gen_code(file, node->l);
        emit2(file, PUSH, QUAD, AX);
        /*
        fprintf(file, "  push rax\n");
        */
        gen_code(file, node->r);
        emit2(file, POP, QUAD, DX);
        emit3(file, ADD, QUAD, DX, AX);
        /*
        fprintf(file, "  pop rdx\n");
        fprintf(file, "  add rax, rdx\n");
        */
        break;

    case NOD_SUB:
        gen_code(file, node->l);
        fprintf(file, "  push rax\n");
        gen_code(file, node->r);
        fprintf(file, "  mov rdx, rax\n");
        fprintf(file, "  pop rax\n");
        fprintf(file, "  sub rax, rdx\n");
        break;

    case NOD_MUL:
        gen_code(file, node->l);
        fprintf(file, "  push rax\n");
        gen_code(file, node->r);
        fprintf(file, "  pop rdx\n");
        fprintf(file, "  imul rax, rdx\n");
        break;

    case NOD_DIV:
        gen_code(file, node->l);
        fprintf(file, "  push rax\n");
        gen_code(file, node->r);
        fprintf(file, "  mov rdi, rax\n");
        fprintf(file, "  pop rax\n");
        fprintf(file, "  cqo\n");
        fprintf(file, "  idiv rdi\n");
        break;

    case NOD_LT:
        gen_code(file, node->l);
        fprintf(file, "  push rax\n");
        gen_code(file, node->r);
        fprintf(file, "  mov rdx, rax\n");
        fprintf(file, "  pop rax\n");
        fprintf(file, "  cmp rax, rdx\n");
        fprintf(file, "  setl al\n");
        fprintf(file, "  movzx rax, al\n");
        break;

    case NOD_GT:
        gen_code(file, node->l);
        fprintf(file, "  push rax\n");
        gen_code(file, node->r);
        fprintf(file, "  mov rdx, rax\n");
        fprintf(file, "  pop rax\n");
        fprintf(file, "  cmp rax, rdx\n");
        fprintf(file, "  setg al\n");
        fprintf(file, "  movzx rax, al\n");
        break;

    case NOD_LE:
        gen_code(file, node->l);
        fprintf(file, "  push rax\n");
        gen_code(file, node->r);
        fprintf(file, "  mov rdx, rax\n");
        fprintf(file, "  pop rax\n");
        fprintf(file, "  cmp rax, rdx\n");
        fprintf(file, "  setle al\n");
        fprintf(file, "  movzx rax, al\n");
        break;

    case NOD_GE:
        gen_code(file, node->l);
        fprintf(file, "  push rax\n");
        gen_code(file, node->r);
        fprintf(file, "  mov rdx, rax\n");
        fprintf(file, "  pop rax\n");
        fprintf(file, "  cmp rax, rdx\n");
        fprintf(file, "  setge al\n");
        fprintf(file, "  movzx rax, al\n");
        break;

    case NOD_EQ:
        gen_code(file, node->l);
        fprintf(file, "  push rax\n");
        gen_code(file, node->r);
        fprintf(file, "  pop rdx\n");
        fprintf(file, "  cmp rax, rdx\n");
        fprintf(file, "  sete al\n");
        fprintf(file, "  movzx rax, al\n");
        break;

    case NOD_NE:
        gen_code(file, node->l);
        fprintf(file, "  push rax\n");
        gen_code(file, node->r);
        fprintf(file, "  pop rdx\n");
        fprintf(file, "  cmp rax, rdx\n");
        fprintf(file, "  setne al\n");
        fprintf(file, "  movzx rax, al\n");
        break;

    default:
        break;
    }
}

void gen_x86(FILE *file, const struct ast_node *tree)
{
    fprintf(file, ".intel_syntax noprefix\n");
    fprintf(file, ".global ");
    print_global_funcs(file, tree);
    fprintf(file, "\n");

    gen_code(file, tree);
}
