#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include "gen_x64.h"
#include "type.h"
#include "esc_seq.h"
#include "x86_64.h"

static int att_syntax = 1;

static const char data_name[][8] = { "?", "byte", "word", "long", "quad" };

static const char *get_data_name(int size)
{
    return data_name[size];
}

static const char *LABEL_NAME_PREFIX = "LBB";
static const char *STR_LIT_NAME_PREFIX = "L.str";

static enum operand_00 make_label_00(int block_id, int label_id)
{
    static char buf[64] = {'\0'};
    sprintf(buf, "%s%d", LABEL_NAME_PREFIX, block_id);
    return label_00(buf, label_id);
}

static void gen_pc_rel_addr(FILE *fp, const char *name, int label_id)
{
    if (label_id < 0)
        fprintf(fp, "_%s", name);
    else
        fprintf(fp, "_%s_%d", name, label_id);
}

static void gen_symbol_name(FILE *fp, const char *prefix, int block_id, int label_id)
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

static int operand_size_00(const struct data_type *type)
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

static enum operand_00 register_from_type(enum operand_00 oper, const struct data_type *type)
{
    const int size = operand_size_00(type);
    return regi_00(oper, size);
}

static int get_mem_offset(const struct ast_node *node)
{
    return node->sym->mem_offset;
}

/* forward declaration */
static void gen_code(FILE *fp, const struct ast_node *node);
static void gen_address(FILE *fp, const struct ast_node *node);
static void gen_load_00(FILE *fp, const struct ast_node *node,
        enum operand_00 addr, enum operand_00 regist);
static void gen_cast(FILE *fp, const struct ast_node *node);
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
    code3_00(fp, ADD_00, imm_00(byte), RSP_00);
    inc_stack_pointer(byte);
}

static void gen_sub_stack_pointer(FILE *fp, int byte)
{
    if (!byte)
        return;
    code3_00(fp, SUB_00, imm_00(byte), RSP_00);
    dec_stack_pointer(byte);
}

static void gen_func_param_list_variadic_(FILE *fp)
{
    int i;

    for (i = 0; i < 6; i++) {
        const int disp = -8 * (6 - i);
        const int reg_ = regi_00(arg_reg_00(i), I64);
        code3_00(fp, MOV_00, reg_, mem_00(RBP_00, disp));
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

    code3_00(fp, MOV_00, RBP_00, R10_00);
    code3_00(fp, SUB_00, imm_00(sym->mem_offset), R10_00);

    for (i = 0; i < N8; i++) {
        const int reg_ = regi_00(arg_reg_00(r), I64);
        code3_00(fp, MOV_00, reg_, mem_00(R10_00, offset));
        r++;
        offset += 8;
    }

    for (i = 0; i < N4; i++) {
        const int reg_ = regi_00(arg_reg_00(r), I32);
        code3_00(fp, MOV_00, reg_, mem_00(R10_00, offset));
        r++;
        offset += 4;
    }

    return r;
}

static void gen_func_param_list_(FILE *fp, const struct symbol *func_sym)
{
    const struct symbol *sym;
    int stored_reg_count = 0;
    int stack_offset = 16; /* from rbp */

    if (is_large_object(func_sym->type)) {
        gen_comment(fp, "save address to returning value");
        code2_00(fp, PUSH_00, RDI_00);
        stored_reg_count++;
    }

    for (sym = first_param(func_sym); sym; sym = next_param(sym)) {
        const int param_size = get_size(sym->type);

        if (is_ellipsis(sym))
            break;

        if (param_size <= 8 && stored_reg_count < 6) {
            const int size = operand_size_00(sym->type);
            const int reg_ = regi_00(arg_reg_00(stored_reg_count), size);
            const int disp = -1 * sym->mem_offset;

            code3_00(fp, MOV_00, reg_, mem_00(RBP_00, disp));
            stored_reg_count++;
        }
        else if (param_size <= 16 && stored_reg_count < 5) {
            stored_reg_count = gen_store_param(fp, sym, stored_reg_count);
        }
        else {
            gen_comment(fp, "save rdi and rdx arg");
            code3_00(fp, MOV_00, RDI_00, R10_00);
            code3_00(fp, MOV_00, RDX_00, R11_00);

            /* src from stack */
            code3_00(fp, MOV_00, RBP_00, RAX_00);
            code3_00(fp, ADD_00, imm_00(stack_offset), RAX_00);
            /* dst from local */
            code3_00(fp, MOV_00, RBP_00, RDX_00);
            code3_00(fp, SUB_00, imm_00(sym->mem_offset), RDX_00);
            gen_assign_struct(fp, sym->type);

            gen_comment(fp, "restore rdi and rdx arg");
            code3_00(fp, MOV_00, R10_00, RDI_00);
            code3_00(fp, MOV_00, R11_00, RDX_00);

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
    assert(ident);

    if (!is_static(ident->sym))
        fprintf(fp, "    .global _%s\n", ident->sym->name);
    fprintf(fp, "_%s:\n", ident->sym->name);
    code2_00(fp, PUSH_00, RBP_00);
    code3_00(fp, MOV_00,  RSP_00, RBP_00);
    code3_00(fp, SUB_00, imm_00(get_local_area_size()), RSP_00);
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
    code3_00(fp, MOV_00, RBP_00, RSP_00);
    code2_00(fp, POP_00, RBP_00);
    code1_00(fp, RET_00);
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
    code3_00(fp, MOV_00, RSP_00, RDX_00);
    code3_00(fp, ADD_00, imm_00(arg->offset), RDX_00);
    gen_assign_struct(fp, arg->expr->type);
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
        const int reg_ = regi_00(arg_reg_00(r), I64);
        code3_00(fp, MOV_00, mem_00(RSP_00, arg->offset + offset), reg_);
        r++;
        offset += 8;
    }

    for (i = 0; i < N4; i++) {
        const int reg_ = regi_00(arg_reg_00(r), I32);
        code3_00(fp, MOV_00, mem_00(RSP_00, arg->offset + offset), reg_);
        r++;
        offset += 4;
    }

    return r;
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
                gen_cast(fp, args[i].expr);
                code3_00(fp, MOV_00, RAX_00, mem_00(RSP_00, args[i].offset));
            }
        }

        /* load to registers */
        gen_comment(fp, "load args");
        for (i = 0; i < arg_count && loaded_reg_count < 6; i++) {
            struct arg_area *ar = &args[i];

            if (!ar->expr) {
                /* large return value */
                const int offset = -get_local_area_size();
                const int reg_ = regi_00(arg_reg_00(0), I64);
                gen_comment(fp, "load address to returned value");
                code3_00(fp, LEA_00, mem_00(RBP_00, offset), reg_);
                loaded_reg_count++;
                continue;
            }

            if (!ar->pass_by_reg)
                continue;

            if (ar->size > 8) {
                loaded_reg_count = gen_load_arg(fp, ar, loaded_reg_count);
            } else {
                const int reg_ = regi_00(arg_reg_00(loaded_reg_count), I64);
                code3_00(fp, MOV_00, mem_00(RSP_00, ar->offset), reg_);
                loaded_reg_count++;
            }
        }

        /* number of fp */
        if (is_variadic(func_sym))
            code3_00(fp, MOV_00, imm_00(0), EAX_00);

        /* call */
        gen_comment(fp, "call");
        code2_00(fp, CALL_00, label_00(func_sym->name, -1));

        gen_comment(fp, "free up arg area");
        gen_add_stack_pointer(fp, total_area_size);
    }

    free(args);

    if (is_medium_object(func_sym->type)) {
        const int offset = -get_local_area_size();

        gen_comment(fp, "store returned value");
        code3_00(fp, MOV_00, RAX_00, mem_00(RBP_00, offset));
        code3_00(fp, MOV_00, RDX_00, mem_00(RBP_00, offset + 8));
        gen_comment(fp, "load address to returned value");
        code3_00(fp, LEA_00, mem_00(RBP_00, offset), RAX_00);
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
        code3_00(fp, SUB_00, imm_00(8), RSP_00);
        code3_00(fp, MOV_00, RAX_00, mem_00(RSP_00, 0));
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
    gen_symbol_name(fp, LABEL_NAME_PREFIX, block_id, label_id);
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
            code3_00(fp, LEA_00, symb_00(sym->name, id), RAX_00);
        } else {
            /* TODO come up with better idea */
            if (!strcmp(sym->name, "__stdinp") ||
                !strcmp(sym->name, "__stdoutp") ||
                !strcmp(sym->name, "__stderrp")) {
                const int a_ = regi_00(A__00, I64);
                char buf[128] = {'\0'};
                sprintf(buf, "%s@GOTPCREL", sym->name);
                code3_00(fp, MOV_00, symb_00(buf, -1), a_);
                code3_00(fp, MOV_00, mem_00(RAX_00, 0), RAX_00);
            } else {
                const int size = operand_size_00(node->type);
                const int a_ = regi_00(A__00, size);
                code3_00(fp, MOV_00, symb_00(sym->name, id), a_);
            }
        }
    }
    else if (is_enumerator(sym)) {
        code3_00(fp, MOV_00, imm_00(get_mem_offset(node)), EAX_00);
    }
    else {
        const int disp = -get_mem_offset(node);

        if (is_array(node->type))
            code3_00(fp, LEA_00, mem_00(RBP_00, disp), RAX_00);
        else
            gen_load_00(fp, node, mem_00(RBP_00, disp), A__00);
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
        code3_00(fp, LEA_00, symb_00(sym->name, id), RAX_00);
    } else {
        code3_00(fp, MOV_00, RBP_00, RAX_00);
        code3_00(fp, SUB_00, imm_00(get_mem_offset(node)), RAX_00);
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
            code3_00(fp, ADD_00, imm_00(disp), RAX_00);
        }
        break;

    default:
        gen_comment(fp, "not an lvalue");
        break;
    }
}

static void gen_load_00(FILE *fp, const struct ast_node *node,
        enum operand_00 addr, enum operand_00 regist)
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
    code3_00(fp, MOV_00, addr, reg_);

    if (is_char(node->type) || is_short(node->type)) {
        gen_comment(fp, "cast");
        gen_cast(fp, node);
    }
}

static void gen_store_00(FILE *fp, const struct ast_node *node,
        enum operand_00 regist, enum operand_00 addr)
{
    const int reg_ = register_from_type(regist, node->type);

    gen_comment(fp, "store");
    if (is_char(node->type) || is_short(node->type) || is_int(node->type)) {
        gen_comment(fp, "cast");
        gen_cast(fp, node);
    }

    code3_00(fp, MOV_00, reg_, addr);
}

static void gen_div(FILE *fp, const struct ast_node *node, enum operand_00 divider)
{
    const int d_ = register_from_type(D__00, node->type);
    const int divider_ = register_from_type(divider, node->type);

    /* rax -> rdx:rax (zero extend) */
    if (is_unsigned(node->type)) {
        code3_00(fp, XOR_00, d_, d_);
        code2_00(fp, DIV_00, divider_);
        return;
    }

    /* rax -> rdx:rax (signed extend) */
    if (is_long(node->type))
        code1_00(fp, CQTO_00);
    else
        code1_00(fp, CLTD_00);

    code2_00(fp, IDIV_00, divider_);
}

static void gen_preincdec(FILE *fp, const struct ast_node *node, enum opecode_00 op)
{
    const int size = operand_size_00(node->type);
    const int a_ = regi_00(A__00, size);
    const int d_ = regi_00(D__00, size);

    int stride = 1;
    if (is_pointer(node->type))
        stride = get_size(underlying(node->type));

    gen_address(fp, node->l);
    code3_00(fp, MOV_00, mem_00(RAX_00, 0), d_);
    code3_00(fp, op, imm_00(stride), d_);
    code3_00(fp, MOV_00, d_, mem_00(RAX_00, 0));
    code3_00(fp, MOV_00, d_, a_);
}

static void gen_postincdec(FILE *fp, const struct ast_node *node, enum opecode_00 op)
{
    const int size = operand_size_00(node->type);
    const int a_ = regi_00(A__00, size);
    const int c_ = regi_00(C__00, size);

    int stride = 1;
    if (is_pointer(node->type))
        stride = get_size(underlying(node->type));

    gen_address(fp, node->l);
    code3_00(fp, MOV_00, RAX_00, RDX_00);
    code3_00(fp, MOV_00, mem_00(RAX_00, 0), a_);
    code3_00(fp, MOV_00, a_, c_);
    code3_00(fp, op, imm_00(stride), c_);
    code3_00(fp, MOV_00, c_, mem_00(RDX_00, 0));
}

static void gen_relational(FILE *fp, const struct ast_node *node, enum opecode_00 op)
{
    const int a_ = register_from_type(A__00, node->type);
    const int d_ = register_from_type(D__00, node->type);

    gen_code(fp, node->l);
    code2_00(fp, PUSH_00, RAX_00);
    gen_code(fp, node->r);
    code3_00(fp, MOV_00, a_, d_);
    code2_00(fp, POP_00, RAX_00);
    code3_00(fp, CMP_00, d_, a_);
    code2_00(fp, op, AL_00);
    code3_00(fp, MOVZB_00, AL_00, a_);
}

static void gen_equality(FILE *fp, const struct ast_node *node, enum opecode_00 op)
{
    const int a_ = register_from_type(A__00, node->type);
    const int d_ = register_from_type(D__00, node->type);

    gen_code(fp, node->l);
    code2_00(fp, PUSH_00, RAX_00);
    gen_code(fp, node->r);
    code2_00(fp, POP_00, RDX_00);
    code3_00(fp, CMP_00, d_, a_);
    code2_00(fp, op,   AL_00);
    code3_00(fp, MOVZB_00, AL_00, a_);
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
            const int a_ = register_from_type(A__00, node->type);

            code3_00(fp, CMP_00, imm_00(node->l->ival), a_);
            code2_00(fp, JE_00,  make_label_00(switch_scope, jump_id(node)));
            /* check next statement if it is another case statement */
            gen_switch_table_(fp, node->r, switch_scope, ctrl_type);
        }
        return;

    case NOD_DEFAULT:
        code2_00(fp, JMP_00, make_label_00(switch_scope, jump_id(node)));
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
    code2_00(fp, JMP_00, make_label_00(switch_scope, JMP_EXIT));
    gen_comment(fp, "end jump table");
}

static void gen_cast(FILE *fp, const struct ast_node *node)
{
    struct data_type *type = node->type;

    if (is_pointer(type))
        return;

    if (is_char(type)) {
        if (is_unsigned(type))
            code3_00(fp, MOVZB_00, AL_00, RAX_00);
        else
            code3_00(fp, MOVSB_00, AL_00, RAX_00);
    }
    else if (is_short(type)) {
        if (is_unsigned(type))
            code3_00(fp, MOVZW_00, AX_00, RAX_00);
        else
            code3_00(fp, MOVSW_00, AX_00, RAX_00);
    }
    else if (is_int(type)) {
        if (is_unsigned(type))
            code3_00(fp, MOV_00,   EAX_00, EAX_00);
        else
            code3_00(fp, MOVSL_00, EAX_00, RAX_00);
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
        code3_00(fp, MOV_00, mem_00(RAX_00, offset), RDI_00);
        code3_00(fp, MOV_00, RDI_00, mem_00(RDX_00, offset));
        offset += 8;
    }
    for (i = 0; i < N4; i++) {
        code3_00(fp, MOV_00, mem_00(RAX_00, offset), EDI_00);
        code3_00(fp, MOV_00, EDI_00, mem_00(RDX_00, offset));
        offset += 4;
    }
}

static void gen_init_scalar_local(FILE *fp, const struct data_type *type,
        const struct ast_node *ident, int offset, const struct ast_node *expr)
{
    gen_comment(fp, "local scalar init");

    /* ident */
    gen_address(fp, ident);
    if (offset > 0)
        code3_00(fp, ADD_00, imm_00(offset), RAX_00);
    code2_00(fp, PUSH_00, RAX_00);

    if (expr) {
        const int a_ = register_from_type(A__00, type);

        /* init expr */
        gen_code(fp, expr);
        /* assign expr */
        code2_00(fp, POP_00,  RDX_00);
        gen_comment(fp, "assign object");
        if (is_small_object(type))
            code3_00(fp, MOV_00, a_, mem_00(RDX_00, 0));
        else
            gen_assign_struct(fp, type);
    } else {
        const int d_ = register_from_type(D__00, type);
        /* assign zero */
        /* need pop to align */
        code2_00(fp, POP_00,  RAX_00);
        code3_00(fp, MOV_00, imm_00(0), d_);
        code3_00(fp, MOV_00, d_, mem_00(RAX_00, 0));
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
    char type_name[128] = {'\0'};
    int i;

    make_type_name(obj->sym->type, type_name);
    printf("%s %s:\n", type_name, obj->sym->name);
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
    else if (is_union(type)) {
        /* initialize only the first member of union */
        const struct symbol *sym = first_member(type->sym);
        zero_clear_bytes(bytes, sym->type);
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
        const struct data_type *type, const struct ast_node *expr)
{
    if (!expr)
        return;

    switch (expr->kind) {

    case NOD_INIT:
        {
            /* move cursor by designator */
            const int offset = expr->ival;

            assign_init(base, type, expr->l);
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
        } else {
            /* assign initializer to byte */
            const int size = base->written_size;
            int i;

            base->init = expr;

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
    const int size = operand_size_00(type);
    const char *szname = get_data_name(size);

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

    int a_, c_, d_, di_;

    if (node == NULL)
        return;

    a_  = register_from_type(A__00, node->type);
    c_  = register_from_type(C__00, node->type);
    d_  = register_from_type(D__00, node->type);
    di_ = register_from_type(DI__00, node->type);

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
        code3_00(fp, CMP_00, imm_00(0), a_);
        code2_00(fp, JE_00, make_label_00(scope.curr, JMP_EXIT));
        break;

    case NOD_FOR_BODY_POST:
        /* body */
        gen_comment(fp, "for-body");
        gen_code(fp, node->l);
        /* post */
        gen_comment(fp, "for-post");
        gen_label(fp, scope.curr, JMP_CONTINUE);
        gen_code(fp, node->r);
        code2_00(fp, JMP_00, make_label_00(scope.curr, JMP_ENTER));
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
        code3_00(fp, CMP_00, imm_00(0), a_);
        code2_00(fp, JE_00,  make_label_00(scope.curr, JMP_EXIT));
        gen_comment(fp, "while-body");
        gen_code(fp, node->r);
        code2_00(fp, JMP_00, make_label_00(scope.curr, JMP_CONTINUE));
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
        code3_00(fp, CMP_00, imm_00(0), a_);
        code2_00(fp, JE_00,  make_label_00(scope.curr, JMP_EXIT));
        code2_00(fp, JMP_00, make_label_00(scope.curr, JMP_ENTER));
        gen_label(fp, scope.curr, JMP_EXIT);

        scope = tmp;
        break;

    case NOD_IF:
        /* if */
        tmp = scope;
        scope.curr = next_scope++;

        gen_comment(fp, "if-cond");
        gen_code(fp, node->l);
        code3_00(fp, CMP_00, imm_00(0), a_);
        code2_00(fp, JE_00,  make_label_00(scope.curr, JMP_ELSE));
        gen_code(fp, node->r);

        scope = tmp;
        break;

    case NOD_IF_THEN:
        /* then */
        gen_comment(fp, "if-then");
        gen_code(fp, node->l);
        code2_00(fp, JMP_00, make_label_00(scope.curr, JMP_EXIT));
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
            code3_00(fp, MOV_00, RAX_00, RSI_00);
            code3_00(fp, MOV_00, mem_00(RSI_00, 0), RAX_00);
            code3_00(fp, MOV_00, mem_00(RSI_00, 8), RDX_00);
        }
        else if (is_large_object(node->type)) {
            gen_comment(fp, "fill returning value");
            code2_00(fp, POP_00, RDX_00);
            gen_assign_struct(fp, node->type);
            gen_comment(fp, "load address to returning value");
            code3_00(fp, MOV_00, RDX_00, RAX_00);
        }

        code2_00(fp, JMP_00, make_label_00(scope.func, JMP_RETURN));
        break;

    case NOD_BREAK:
        code2_00(fp, JMP_00, make_label_00(scope.brk, JMP_EXIT));
        break;

    case NOD_CONTINUE:
        code2_00(fp, JMP_00, make_label_00(scope.conti, JMP_CONTINUE));
        break;

    case NOD_LABEL:
        gen_label(fp, scope.func, jump_id(node->l));
        gen_code(fp, node->r);
        break;

    case NOD_GOTO:
        code2_00(fp, JMP_00, make_label_00(scope.func, jump_id(node->l)));
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
        gen_load_00(fp, node, mem_00(RAX_00, 0), A__00);
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
        code2_00(fp, PUSH_00, RAX_00);
        gen_code(fp, node->r);
        code2_00(fp, POP_00,  RDX_00);

        if (0)
            gen_store_00(fp, node->r, A__00, mem_00(RDX_00, 0));

        /* TODO come up with better idea to cast when storing */
        if (is_long(node->type) && !is_long(node->r->type))
            gen_cast(fp, node->r);

        if (is_small_object(node->type))
            code3_00(fp, MOV_00, a_, mem_00(RDX_00, 0));
        else
            gen_assign_struct(fp, node->type);

        break;

    case NOD_ADD_ASSIGN:
        gen_comment(fp, "add-assign");
        gen_address(fp, node->l);
        code2_00(fp, PUSH_00, RAX_00);
        gen_code(fp, node->r);
        code2_00(fp, POP_00,  RDX_00);
        code3_00(fp, ADD_00, a_, mem_00(RDX_00, 0));
        code3_00(fp, MOV_00, mem_00(RDX_00, 0), a_);
        break;

    case NOD_SUB_ASSIGN:
        gen_comment(fp, "sub-assign");
        gen_address(fp, node->l);
        code2_00(fp, PUSH_00, RAX_00);
        gen_code(fp, node->r);
        code2_00(fp, POP_00,  RDX_00);
        code3_00(fp, SUB_00, a_, mem_00(RDX_00, 0));
        code3_00(fp, MOV_00, mem_00(RDX_00, 0), a_);
        break;

    case NOD_MUL_ASSIGN:
        gen_comment(fp, "mul-assign");
        gen_address(fp, node->l);
        code2_00(fp, PUSH_00, RAX_00);
        gen_code(fp, node->r);
        code2_00(fp, POP_00,  RDX_00);
        code3_00(fp, IMUL_00, mem_00(RDX_00, 0), a_);
        code3_00(fp, MOV_00, a_, mem_00(RDX_00, 0));
        break;

    case NOD_DIV_ASSIGN:
        gen_comment(fp, "div-assign");
        gen_address(fp, node->l);
        code2_00(fp, PUSH_00, RAX_00);
        gen_code(fp, node->r);
        code3_00(fp, MOV_00, a_, di_);
        code2_00(fp, POP_00, RSI_00);
        code3_00(fp, MOV_00, mem_00(RSI_00, 0), a_);
        /* rax -> rdx:rax */
        code1_00(fp, CLTD_00);
        code2_00(fp, IDIV_00, di_);
        code3_00(fp, MOV_00, a_, mem_00(RSI_00, 0));
        break;

    case NOD_MOD_ASSIGN:
        gen_comment(fp, "mod-assign");
        gen_address(fp, node->l);
        code2_00(fp, PUSH_00, RAX_00);
        gen_code(fp, node->r);
        code3_00(fp, MOV_00, a_, di_);
        code2_00(fp, POP_00, RSI_00);
        code3_00(fp, MOV_00, mem_00(RSI_00, 0), a_);
        gen_div(fp, node, DI__00);
        code3_00(fp, MOV_00, d_, a_);
        code3_00(fp, MOV_00, a_, mem_00(RSI_00, 0));
        break;

    case NOD_SHL_ASSIGN:
        gen_comment(fp, "shl-assign");
        gen_address(fp, node->l);
        code2_00(fp, PUSH_00, RAX_00);
        gen_code(fp, node->r);
        code3_00(fp, MOV_00, a_, c_);
        code2_00(fp, POP_00, RDX_00);
        code3_00(fp, MOV_00, mem_00(RDX_00, 0), a_);
        code3_00(fp, SHL_00, CL_00, a_);
        code3_00(fp, MOV_00, a_, mem_00(RDX_00, 0));
        break;

    case NOD_SHR_ASSIGN:
        gen_comment(fp, "shr-assign");
        gen_address(fp, node->l);
        code2_00(fp, PUSH_00, RAX_00);
        gen_code(fp, node->r);
        code3_00(fp, MOV_00, a_, c_);
        code2_00(fp, POP_00, RDX_00);
        code3_00(fp, MOV_00, mem_00(RDX_00, 0), a_);
        if (is_unsigned(node->type))
            code3_00(fp, SHR_00, CL_00, a_);
        else
            code3_00(fp, SAR_00, CL_00, a_);
        code3_00(fp, MOV_00, a_, mem_00(RDX_00, 0));
        break;

    case NOD_OR_ASSIGN:
        gen_comment(fp, "or-assign");
        gen_address(fp, node->l);
        code2_00(fp, PUSH_00, RAX_00);
        gen_code(fp, node->r);
        code2_00(fp, POP_00, RDX_00);
        code3_00(fp, OR_00, a_, mem_00(RDX_00, 0));
        code3_00(fp, MOV_00, mem_00(RDX_00, 0), a_);
        break;

    case NOD_XOR_ASSIGN:
        gen_comment(fp, "or-assign");
        gen_address(fp, node->l);
        code2_00(fp, PUSH_00, RAX_00);
        gen_code(fp, node->r);
        code2_00(fp, POP_00, RDX_00);
        code3_00(fp, XOR_00, a_, mem_00(RDX_00, 0));
        code3_00(fp, MOV_00, mem_00(RDX_00, 0), a_);
        break;

    case NOD_AND_ASSIGN:
        gen_comment(fp, "or-assign");
        gen_address(fp, node->l);
        code2_00(fp, PUSH_00, RAX_00);
        gen_code(fp, node->r);
        code2_00(fp, POP_00,  RDX_00);
        code3_00(fp, AND_00, a_, mem_00(RDX_00, 0));
        code3_00(fp, MOV_00, mem_00(RDX_00, 0), a_);
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
        gen_load_00(fp, node, mem_00(RAX_00, 0), A__00);
        break;

    case NOD_NUM:
        code3_00(fp, MOV_00, imm_00(node->ival), a_);
        break;

    case NOD_STRING:
        code3_00(fp, LEA_00, symb_00(STR_LIT_NAME_PREFIX, node->sym->id), RAX_00);
        break;

    case NOD_SIZEOF:
        code3_00(fp, MOV_00, imm_00(get_size(node->l->type)), EAX_00);
        break;

    case NOD_ADD:
        gen_code(fp, node->l);
        code2_00(fp, PUSH_00, RAX_00);
        gen_code(fp, node->r);
        code2_00(fp, POP_00, RDX_00);

        /* TODO find the best place to handle array subscript */
        if (is_array(node->l->type) || is_pointer(node->l->type)) {
            const int sz = get_size(underlying(node->l->type));
            code3_00(fp, IMUL_00, imm_00(sz), RAX_00);
        }

        code3_00(fp, ADD_00, d_, a_);
        break;

    case NOD_SUB:
        gen_code(fp, node->l);
        code2_00(fp, PUSH_00, RAX_00);
        gen_code(fp, node->r);
        code3_00(fp, MOV_00, a_, d_);
        code2_00(fp, POP_00, RAX_00);
        code3_00(fp, SUB_00, d_, a_);
        break;

    case NOD_MUL:
        gen_code(fp, node->l);
        code2_00(fp, PUSH_00, RAX_00);
        gen_code(fp, node->r);
        code2_00(fp, POP_00, RDX_00);
        code3_00(fp, IMUL_00, d_, a_);
        break;

    case NOD_DIV:
        gen_code(fp, node->l);
        code2_00(fp, PUSH_00, RAX_00);
        gen_code(fp, node->r);
        code3_00(fp, MOV_00, a_, di_);
        code2_00(fp, POP_00, RAX_00);
        /* rax -> rdx:rax */
        code1_00(fp, CLTD_00);
        code2_00(fp, IDIV_00, di_);
        break;

    case NOD_MOD:
        gen_code(fp, node->l);
        code2_00(fp, PUSH_00, RAX_00);
        gen_code(fp, node->r);
        code3_00(fp, MOV_00, a_, di_);
        code2_00(fp, POP_00, RAX_00);
        gen_div(fp, node, DI__00);
        code3_00(fp, MOV_00, d_, a_);
        break;

    case NOD_SHL:
        gen_code(fp, node->l);
        code2_00(fp, PUSH_00, RAX_00);
        gen_code(fp, node->r);
        code3_00(fp, MOV_00, a_, c_);
        code2_00(fp, POP_00, RAX_00);
        code3_00(fp, SHL_00, CL_00, a_);
        break;

    case NOD_SHR:
        gen_code(fp, node->l);
        code2_00(fp, PUSH_00, RAX_00);
        gen_code(fp, node->r);
        code3_00(fp, MOV_00, a_, c_);
        code2_00(fp, POP_00, RAX_00);
        if (is_unsigned(node->type))
            code3_00(fp, SHR_00, CL_00, a_);
        else
            code3_00(fp, SAR_00, CL_00, a_);
        break;

    case NOD_OR:
        gen_code(fp, node->l);
        code2_00(fp, PUSH_00, RAX_00);
        gen_code(fp, node->r);
        code2_00(fp, POP_00, RDX_00);
        code3_00(fp, OR_00, d_, a_);
        break;

    case NOD_XOR:
        gen_code(fp, node->l);
        code2_00(fp, PUSH_00, RAX_00);
        gen_code(fp, node->r);
        code2_00(fp, POP_00, RDX_00);
        code3_00(fp, XOR_00, d_, a_);
        break;

    case NOD_AND:
        gen_code(fp, node->l);
        code2_00(fp, PUSH_00, RAX_00);
        gen_code(fp, node->r);
        code2_00(fp, POP_00, RDX_00);
        code3_00(fp, AND_00, d_, a_);
        break;

    case NOD_NOT:
        gen_code(fp, node->l);
        code2_00(fp, NOT_00, a_);
        break;

    case NOD_COND:
        /* cond */
        gen_comment(fp, "cond-?");
        tmp = scope;
        scope.curr = next_scope++;

        gen_code(fp, node->l);
        code3_00(fp, CMP_00, imm_00(0), a_);
        code2_00(fp, JE_00,  make_label_00(scope.curr, JMP_ELSE));
        gen_code(fp, node->r);

        scope = tmp;
        break;

    case NOD_COND_THEN:
        /* then */
        gen_comment(fp, "cond-then");
        gen_code(fp, node->l);
        code2_00(fp, JMP_00, make_label_00(scope.curr, JMP_EXIT));
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
        code3_00(fp, CMP_00, imm_00(0), a_);
        code2_00(fp, JNE_00, make_label_00(scope.curr, JMP_CONTINUE));
        gen_code(fp, node->r);
        code3_00(fp, CMP_00, imm_00(0), a_);
        code2_00(fp, JE_00,  make_label_00(scope.curr, JMP_EXIT));
        gen_label(fp, scope.curr, JMP_CONTINUE);
        code3_00(fp, MOV_00, imm_00(1), EAX_00);
        gen_label(fp, scope.curr, JMP_EXIT);

        scope = tmp;
        break;

    case NOD_LOGICAL_AND:
        tmp = scope;
        scope.curr = next_scope++;

        gen_code(fp, node->l);
        code3_00(fp, CMP_00, imm_00(0), a_);
        code2_00(fp, JE_00,  make_label_00(scope.curr, JMP_EXIT));
        gen_code(fp, node->r);
        code3_00(fp, CMP_00, imm_00(0), a_);
        code2_00(fp, JE_00,  make_label_00(scope.curr, JMP_EXIT));
        code3_00(fp, MOV_00, imm_00(1), EAX_00);
        gen_label(fp, scope.curr, JMP_EXIT);

        scope = tmp;
        break;

    case NOD_LOGICAL_NOT:
        gen_code(fp, node->l);
        code3_00(fp, CMP_00, imm_00(0), a_);
        code2_00(fp, SETE_00,  AL_00);
        code3_00(fp, MOVZB_00, AL_00, a_);
        break;

    case NOD_COMMA:
        gen_code(fp, node->l);
        gen_code(fp, node->r);
        break;

    case NOD_PREINC:
        gen_preincdec(fp, node, ADD_00);
        break;

    case NOD_PREDEC:
        gen_preincdec(fp, node, SUB_00);
        break;

    case NOD_POSTINC:
        gen_postincdec(fp, node, ADD_00);
        break;

    case NOD_POSTDEC:
        gen_postincdec(fp, node, SUB_00);
        break;

    case NOD_LT:
        gen_relational(fp, node, SETL_00);
        break;

    case NOD_GT:
        gen_relational(fp, node, SETG_00);
        break;

    case NOD_LE:
        gen_relational(fp, node, SETLE_00);
        break;

    case NOD_GE:
        gen_relational(fp, node, SETGE_00);
        break;

    case NOD_EQ:
        gen_equality(fp, node, SETE_00);
        break;

    case NOD_NE:
        gen_equality(fp, node, SETNE_00);
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
