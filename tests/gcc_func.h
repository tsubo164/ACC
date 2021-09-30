#ifndef GCC_FUNC_H
#define GCC_FUNC_H

typedef struct point {
    int x, y;
} point;

typedef struct vec {
    int x, y, z, w;
} vec;

int gcc_twice(int a);
void gcc_add(char a, short b, int c, long d, long *out);
int gcc_sum1234_mult_sum5678(
        int a1, int a2, int a3, int a4,
        int a5, int a6, int a7, int a8);

int gcc_get_x(point p);
int gcc_get_y(point p);

int gcc_get_x4(vec v);
int gcc_get_y4(vec v);
int gcc_get_z4(vec v);
int gcc_get_w4(vec v);

#endif /* GCC_FUNC_H */
