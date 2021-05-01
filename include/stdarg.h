#ifndef __STDARG_H
#define __STDARG_H

typedef struct {
    unsigned int gp_offset;
    unsigned int fp_offset;
    void *overflow_arg_area;
    void *reg_save_area;
} va_list[1];

void __builtin_va_start(va_list ap, void *last);

#define va_start(ap,last) __builtin_va_start((ap), &(last))
#define va_end(ap)

#endif /* __STDARG_H */
