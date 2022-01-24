#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "type.h"
#include "symbol.h"

#define UNKNOWN_ARRAY_LENGTH -1

/*                                                       Sz Al Ln Ul    Sy  */
static struct data_type VOID_     = {DATA_TYPE_VOID,     1, 4, 1, NULL, NULL};
static struct data_type CHAR_     = {DATA_TYPE_CHAR,     1, 1, 1, NULL, NULL};
static struct data_type SHORT_    = {DATA_TYPE_SHORT,    2, 2, 1, NULL, NULL};
static struct data_type INT_      = {DATA_TYPE_INT,      4, 4, 1, NULL, NULL};
static struct data_type LONG_     = {DATA_TYPE_LONG,     8, 8, 1, NULL, NULL};
static struct data_type POINTER_  = {DATA_TYPE_POINTER,  8, 8, 1, NULL, NULL};
static struct data_type ARRAY_    = {DATA_TYPE_ARRAY,    0, 0, UNKNOWN_ARRAY_LENGTH, NULL, NULL};
static struct data_type STRUCT_   = {DATA_TYPE_STRUCT,   1, 1, 1, NULL, NULL};
static struct data_type UNION_    = {DATA_TYPE_UNION,    1, 1, 1, NULL, NULL};
static struct data_type ENUM_     = {DATA_TYPE_ENUM,     4, 4, 1, NULL, NULL};
static struct data_type FUNCTION_ = {DATA_TYPE_FUNCTION, 4, 4, 1, NULL, NULL};
static struct data_type PLACEHOLDER_ = {DATA_TYPE_PLACEHOLDER, 0, 0, 0, NULL, NULL};

int get_size(const struct data_type *type)
{
    if (is_array(type))
        return get_array_length(type) * get_size(underlying(type));
    return type->byte_size;
}

int get_alignment(const struct data_type *type)
{
    return type->alignment;
}

int get_array_length(const struct data_type *type)
{
    return type->array_len;
}

struct data_type *underlying(const struct data_type *type)
{
    if (!type)
        return NULL;
    return type->base;
}

struct symbol *symbol_of(const struct data_type *type)
{
    return type->sym;
}

void set_array_length(struct data_type *type, int len)
{
    if (!is_array(type))
        return;
    type->array_len = len;
}

void set_struct_size(struct data_type *type, int size)
{
    if (!is_struct(type))
        return;
    type->byte_size = size;
}

void set_struct_align(struct data_type *type, int align)
{
    if (!is_struct(type))
        return;
    type->alignment = align;
}

void set_union_size(struct data_type *type, int size)
{
    if (!is_union(type))
        return;
    type->byte_size = size;
}

void set_union_align(struct data_type *type, int align)
{
    if (!is_union(type))
        return;
    type->alignment = align;
}

void set_symbol(struct data_type *type, struct symbol *sym)
{
    type->sym = sym;
}

static struct data_type *clone(const struct data_type *orig)
{
    struct data_type *type = malloc(sizeof(struct data_type));
    *type = *orig;
    return type;
}

static int is_cloned(const struct data_type *type)
{
    return type->is_const || type->is_unsigned;
}

struct data_type *make_const(struct data_type *orig)
{
    struct data_type *type;

    if (!is_cloned(orig))
        type = clone(orig);
    else
        type = orig;

    type->is_const = 1;
    return type;
}

struct data_type *make_unsigned(struct data_type *orig)
{
    struct data_type *type;

    if (!is_cloned(orig))
        type = clone(orig);
    else
        type = orig;

    type->is_unsigned = 1;
    return type;
}

struct data_type *promote(struct data_type *t1, struct data_type *t2)
{
    if (!t1 && !t2)
        return type_void();
    if (!t1)
        return t2;
    if (!t2)
        return t1;

    if (t1->kind == t2->kind) {
        if (is_unsigned(t1))
            return t1;
        else
            return t2;
    }

    if (t1->kind > t2->kind)
        return t1;
    else
        return t2;
}

int has_unkown_array_length(const struct data_type *type)
{
    return get_array_length(type) == UNKNOWN_ARRAY_LENGTH;
}

int has_typedef_name(const struct data_type *type)
{
    return type->alias != NULL;
}

static int is_identical(const struct data_type *t1, const struct data_type *t2)
{
    if (!t1 || !t2)
        return 0;

    if ((is_char(t1) && is_char(t2)) ||
        (is_short(t1) && is_short(t2)) ||
        (is_int(t1) && is_int(t2)) ||
        (is_long(t1) && is_long(t2)))
        return 1;

    if (is_struct(t1) && is_struct(t2))
        return symbol_of(t1) == symbol_of(t2);

    if (is_union(t1) && is_union(t2))
        return symbol_of(t1) == symbol_of(t2);

    if (is_pointer(t1) && is_pointer(t2))
        return is_identical(underlying(t1), underlying(t2));

    if (is_function(t1) && is_function(t2)) {
        const struct parameter *p1, *p2;

        /* checking return type */
        if (!is_identical(underlying(t1), underlying(t2)))
            return 0;

        /* checking parameter types */
        p1 = first_param_(t1);
        p2 = first_param_(t2);

        while (p1 && p2 && is_identical(p1->sym->type, p2->sym->type)) {
            p1 = next_param_(p1);
            p2 = next_param_(p2);
        }
        return !p1 && !p2;
    }

    return 0;
}

int is_compatible(const struct data_type *t1, const struct data_type *t2)
{
    if (!t1 || !t2)
        return 0;

    if (is_integer(t1) && is_integer(t2))
        return 1;

    if (is_struct(t1) && is_struct(t2))
        return is_identical(t1, t2);

    if (is_union(t1) && is_union(t2))
        return is_identical(t1, t2);

    /* 6.3.2.3 Pointers void * <-> T * */
    if (is_pointer(t1) && is_pointer(t2) && is_void(underlying(t2)))
        return 1;
    if (is_pointer(t1) && is_void(underlying(t1)) && is_pointer(t2))
        return 1;

    if (is_pointer(t1) && is_pointer(t2))
        return is_identical(underlying(t1), underlying(t2));

    return 0;
}

int is_incomplete(const struct data_type *type)
{
    if (is_void(type))
        return 1;
    if (is_struct(type) && !type->sym->is_defined)
        return 1;
    if (is_union(type) && !type->sym->is_defined)
        return 1;
    if (is_enum(type) && !type->sym->is_defined)
        return 1;
    return 0;
}

int is_integer(const struct data_type *type)
{
    if (is_char(type) ||
        is_short(type) ||
        is_int(type) ||
        is_long(type))
        return 1;
    if (is_enum(type))
        return 1;
    return 0;
}

int is_const(const struct data_type *type)
{
    return type->is_const;
}

int is_unsigned(const struct data_type *type)
{
    return type->is_unsigned;
}

int is_void(const struct data_type *type)
{
    return type && type->kind == DATA_TYPE_VOID;
}

int is_char(const struct data_type *type)
{
    return type && type->kind == DATA_TYPE_CHAR;
}

int is_short(const struct data_type *type)
{
    return type && type->kind == DATA_TYPE_SHORT;
}

int is_int(const struct data_type *type)
{
    return type && type->kind == DATA_TYPE_INT;
}

int is_long(const struct data_type *type)
{
    return type && type->kind == DATA_TYPE_LONG;
}

int is_pointer(const struct data_type *type)
{
    return type && type->kind == DATA_TYPE_POINTER;
}

int is_array(const struct data_type *type)
{
    return type && type->kind == DATA_TYPE_ARRAY;
}

int is_struct(const struct data_type *type)
{
    return type && type->kind == DATA_TYPE_STRUCT;
}

int is_union(const struct data_type *type)
{
    return type && type->kind == DATA_TYPE_UNION;
}

int is_enum(const struct data_type *type)
{
    return type && type->kind == DATA_TYPE_ENUM;
}

int is_struct_or_union(const struct data_type *type)
{
    return is_struct(type) || is_union(type);
}

int is_function(const struct data_type *type)
{
    return type && type->kind == DATA_TYPE_FUNCTION;
}

int is_placeholder(const struct data_type *type)
{
    return type && type->kind == DATA_TYPE_PLACEHOLDER;
}

struct data_type *swap_placeholder(struct data_type *head, struct data_type *type)
{
    struct data_type *t;

    if (is_placeholder(head))
        return type;

    for (t = head; !is_placeholder(underlying(t)); t = underlying(t))
        ;
    t->base = type;

    return head;
}

static char *make_type_name_(const struct data_type *type, char *buf)
{
    char *p = buf;
    int n = 0;

    if (!type)
        return NULL;

    if (is_const(type)) {
        sprintf(p, "const %n", &n);
        p += n;
    }

    if (is_void(type)) {
        sprintf(p, "void %n", &n);
        p += n;
    }
    else if (is_char(type)) {
        sprintf(p, "char %n", &n);
        p += n;
    }
    else if (is_short(type)) {
        sprintf(p, "short %n", &n);
        p += n;
    }
    else if (is_int(type)) {
        sprintf(p, "int %n", &n);
        p += n;
    }
    else if (is_long(type)) {
        sprintf(p, "long %n", &n);
        p += n;
    }
    else if (is_pointer(type)) {
        /* XXX need improvement to orders to write */
        if (is_array(underlying(type)) || is_function(underlying(type))) {
            sprintf(p, "(*)%n", &n);
            p += n;
            p = make_type_name_(underlying(type), p);
        }
        else {
            p = make_type_name_(underlying(type), p);
            sprintf(p, "*%n", &n);
            p += n;
        }
    }
    else if (is_array(type)) {
        const struct data_type *t;

        /* print underlying type first */
        for (t = type; is_array(t); t = underlying(t))
            ;
        p = make_type_name_(t, p);

        /* print array lengths */
        for (t = type; is_array(t); t = underlying(t)) {
            sprintf(p, "[%d]%n", get_array_length(t), &n);
            p += n;
        }
    }
    else if (is_struct(type)) {
        sprintf(p, "struct %s %n", symbol_of(type)->name, &n);
        p += n;
    }
    else if (is_union(type)) {
        sprintf(p, "union %s %n", symbol_of(type)->name, &n);
        p += n;
    }
    else if (is_enum(type)) {
        sprintf(p, "enum %s %n", symbol_of(type)->name, &n);
        p += n;
    }
    else if (is_function(type)) {
        p = make_type_name_(underlying(type), p);
        sprintf(p++, "(");
        {
            const struct symbol *s = first_param(symbol_of(type));
            if (!s) {
                sprintf(p, "void%n", &n);
                p += n;
            }
            else {
                int i = 0;
                for (; s; s = next_param(s)) {
                    if (*(p-1) == ' ')
                        p--;
                    if (i++ > 0) {
                        *p++ = ',';
                        *p++ = ' ';
                    }

                    if (is_ellipsis(s)) {
                        sprintf(p, "...%n", &n);
                        p += n;
                        break;
                    }
                    else {
                        p = make_type_name_(s->type, p);
                    }
                }
            }
        }
        if (*(p-1) == ' ')
            p--;
        sprintf(p++, ")");
    }

    return p;
}

void make_type_name(const struct data_type *type, char *buf)
{
    char *p;

    if (!type)
        return;

    p = make_type_name_(type, buf);
    /* trim trailing space */
    if (p != buf && *(p-1) == ' ')
        *(p-1) = '\0';
}

void print_data_type(const struct data_type *type)
{
    char type_name[128] = {'\0'};

    if (!type)
        return;

    make_type_name(type, type_name);
    printf("    name:      %s\n", type_name);
    printf("    kind:      %d\n", type->kind);
    printf("    byte_size: %d\n", type->byte_size);
    printf("    array_len: %d\n", type->array_len);
    printf("    base:      %p\n", (void *) type->base);
    printf("    sym:       %p\n", (void *) type->sym);
}

void copy_data_type(struct data_type *dst, const struct data_type *src)
{
    if (!dst || !src || dst == src)
        return;
    *dst = *src;
}

struct data_type *convert_array_to_pointer(struct data_type *type)
{
    if (is_array(type)) {
        struct data_type *base = underlying(type);
        const int is_const = type->is_const;

        base = convert_array_to_pointer(base);

        type = type_pointer(base);
        type->is_const = is_const;
    }

    return type;
}

struct data_type *type_void(void)
{
    return clone(&VOID_);
}

struct data_type *type_char(void)
{
    return clone(&CHAR_);
}

struct data_type *type_short(void)
{
    return clone(&SHORT_);
}

struct data_type *type_int(void)
{
    return clone(&INT_);
}

struct data_type *type_long(void)
{
    return clone(&LONG_);
}

struct data_type *type_pointer(struct data_type *base_type)
{
    struct data_type *type = clone(&POINTER_);
    type->base = base_type;
    return type;
}

struct data_type *type_array(struct data_type *base_type)
{
    struct data_type *type = clone(&ARRAY_);

    type->base = base_type;
    type->byte_size = base_type->byte_size;
    type->alignment = base_type->alignment;
    /* type->array_len will be computed in later phase */

    return type;
}

struct data_type *type_struct(void)
{
    struct data_type *type = clone(&STRUCT_);
    return type;
}

struct data_type *type_union(void)
{
    struct data_type *type = clone(&UNION_);
    return type;
}

struct data_type *type_enum(void)
{
    struct data_type *type = clone(&ENUM_);
    return type;
}

struct data_type *type_type_name(struct symbol *type_name)
{
    return type_name->type;
}

struct data_type *type_function(struct data_type *return_type)
{
    struct data_type *type = clone(&FUNCTION_);
    type->base = return_type;
    return type;
}

struct data_type *type_placeholder(void)
{
    struct data_type *type = clone(&PLACEHOLDER_);
    return type;
}

struct member *new_member(struct symbol *sym)
{
    struct member *memb = NULL;

    if (!sym)
        return NULL;

    memb = calloc(1, sizeof(struct member));
    memb->sym = sym;

    return memb;
}

struct member *append_member(struct member *head, struct member *memb)
{
    struct member *m = NULL;

    if (!head)
        return memb;

    for (m = head; m->next; m = m->next)
        ;
    m->next = memb;

    return head;
}

void add_member_list(struct data_type *type, struct member *head)
{
    if (!is_struct_or_union(type))
        return;
    type->members = head;
}

const struct member *find_member(const struct data_type *type, const char *name)
{
    const struct member *m;

    for (m = first_member_(type); m; m = next_member_(m))
        if (!strcmp(m->sym->name, name))
            break;

    return m;
}

struct parameter *new_parameter(struct symbol *sym)
{
    struct parameter *param = NULL;

    if (!sym)
        return NULL;

    param = calloc(1, sizeof(struct parameter));
    param->sym = sym;

    return param;
}

struct parameter *append_parameter(struct parameter *head, struct parameter *param)
{
    struct parameter *p = NULL;

    if (!head)
        return param;

    for (p = head; p->next; p = p->next)
        ;
    p->next = param;

    return head;
}

void add_parameter_list(struct data_type *type, struct parameter *head)
{
    if (!is_function(type))
        return;
    type->parameters = head;
}

const struct member *first_member_(const struct data_type *type)
{
    if (!is_struct_or_union(type))
        return NULL;

    if (is_incomplete(type))
        return NULL;

    return type->members;
}

static int is_bit_padding(const struct symbol *sym)
{
    return is_bitfield(sym) && !sym->name;
}

const struct member *next_member_(const struct member *memb)
{
    const struct member *m;

    for (m = memb->next; m; m = m->next) {
        if (is_bit_padding(m->sym))
            continue;
        break;
    }

    return m;
}

const struct parameter *first_param_(const struct data_type *type)
{
    if (!is_function(type))
        return NULL;

    return type->parameters;
}

const struct parameter *next_param_(const struct parameter *param)
{
    if (!param)
        return NULL;

    /* TODO reconsider returning NULL */
    if (is_ellipsis(param->sym))
        return param;

    return param->next;
}

static int align_to(int pos, int align)
{
    assert(align > 0);
    return ((pos + align - 1) / align) * align;
}

static int to_bit(int byte)
{
    return 8 * byte;
}

static int to_byte(int bit)
{
    assert(bit % 8 == 0);
    return bit / 8;
}

static void print_bitfield(const struct symbol *sym)
{
    if (!is_bitfield(sym))
        return;
    printf("  %s:\n", sym->name);
    printf("    bit_width:  %d\n", sym->bit_width);
    printf("    bit_offset: %d\n", sym->bit_offset);
    printf("    mem_offset: %d\n", sym->mem_offset);
}

static int compute_offset(const struct data_type *type, int total_offset)
{
    int offset = total_offset;
    const int size  = get_size(type);
    const int align = get_alignment(type);

    offset = align_to(offset, align);
    offset += size;

    return offset;
}

static int compute_param_size(const struct parameter *params, int total_offset, int variadic)
{
    const struct parameter *p;
    int param_index = 0;
    int offset = total_offset;

    for (p = params; p; p = p->next) {
        struct symbol *sym = p->sym;

        if (param_index < 6) {
            if (variadic) {
                /* store to var_list.reg_save_area */
                sym->mem_offset = 8 * (6 - param_index);
            } else {
                offset = compute_offset(sym->type, offset);
                sym->mem_offset = offset;
            }
        } else {
            int stack_index = param_index - 6;
            /* return address and original rbp (8 + 8 = 16 bytes)
             * is on top of arguments */
            sym->mem_offset = -(16 + 8 * stack_index);
        }
        param_index++;
    }

    return offset;
}

void compute_func_size_(struct data_type *type)
{
    struct symbol *func_sym = symbol_of(type);
    struct symbol *sym;
    const int variadic = is_variadic(func_sym);
    int total_offset = 0;

    if (variadic)
        /* the size of reg_save_area (8 bytes * 6 registers) */
        total_offset += 48;

    total_offset = compute_param_size(type->parameters, total_offset, variadic);

    for (sym = func_sym; sym; sym = sym->next) {

        if (is_local_var(sym)) {
            total_offset = compute_offset(sym->type, total_offset);
            sym->mem_offset = total_offset;
        }

        if (sym->kind == SYM_SCOPE_END && sym->scope_level == 0)
            break;
    }

    func_sym->mem_offset = align_to(total_offset, 16);
}

void compute_struct_size_(struct data_type *type)
{
    struct member *memb = NULL;
    /* compute in bit */
    int max_size = 0;
    int max_align = 0;

    if (is_incomplete(type))
        return;

    /* can not use next_member as it skips unnamed bit-field */
    for (memb = type->members; memb; memb = memb->next) {
        struct symbol *sym = memb->sym;

        if (is_bitfield(sym) && sym->bit_width == 0) {
            const int align = to_bit(4); /* 32 bit */
            /* just align to next unit and adds no padding */
            max_size = align_to(max_size, align);
        }
        else if (is_bitfield(sym)) {
            const int size  = sym->bit_width;
            const int align = to_bit(4); /* 32 bit */
            const int fits = max_size % align + size <= align;

            if (!fits)
                max_size = align_to(max_size, align);

            sym->mem_offset = (max_size / align) * 4;
            sym->bit_offset = max_size % align;
            max_size += size;
            max_align = align > max_align ? align : max_align;
        }
        else {
            const int size = to_bit(get_size(sym->type));
            const int align = to_bit(get_alignment(sym->type));

            max_size = align_to(max_size, align);
            sym->mem_offset = to_byte(max_size);
            max_size += size;
            max_align = align > max_align ? align : max_align;
        }
        if (0)
            print_bitfield(sym);
    }

    if (max_align == 0) {
        /* an empty struct created by error. pretends its size is 4 */
        max_size = to_bit(4);
        max_align = to_bit(4);
    }
    /* align to n byte */
    max_size = align_to(max_size, max_align);

    {
        /* convert bits to bytes */
        const int byte_size  = align_to(to_byte(max_size), to_byte(max_align));
        const int byte_align = to_byte(max_align);

        set_struct_size(type, byte_size);
        set_struct_align(type, byte_align);
        symbol_of(type)->mem_offset = byte_size;
    }
}

void compute_union_size_(struct data_type *type)
{
    struct member *memb = NULL;
    int max_size = 0;
    int max_align = 0;

    if (is_incomplete(type))
        return;

    for (memb = type->members; memb; memb = memb->next) {
        struct symbol *sym = memb->sym;
        const int size  = get_size(sym->type);
        const int align = get_alignment(sym->type);

        sym->mem_offset = 0;
        max_size = size > max_size ? size : max_size;
        max_align = align > max_align ? align : max_align;
    }

    if (max_align == 0) {
        /* an empty union created by error. pretends its size is 4 */
        max_size = 4;
        max_align = 4;
    }

    {
        const int union_size  = align_to(max_size, max_align);
        const int union_align = max_align;

        set_union_size(type, union_size);
        set_union_align(type, union_align);
        symbol_of(type)->mem_offset = union_size;
    }
}

void compute_enum_size_(struct data_type *type)
{
    symbol_of(type)->mem_offset = get_size(type);
}

struct data_type *return_type_(const struct data_type *type)
{
    if (!is_function(type))
        return NULL;

    return underlying(type);
}
