#include <kf/semantic_types.h>

/* Psy-Q LIBGTE MATRIX has the KfMatrix layout: MulMatrix(m0, m1), MulMatrix2(m0, m1). */
extern struct KfMatrix *MulMatrix(struct KfMatrix *m0, struct KfMatrix *m1);
extern struct KfMatrix *MulMatrix2(struct KfMatrix *m0, struct KfMatrix *m1);
extern void func_80014b7c(s16 angle, struct KfMatrix *matrix);
extern void func_80014bec(s16 angle, struct KfMatrix *matrix);
extern void func_80014c5c(s16 angle, struct KfMatrix *matrix);

void func_80014ccc(const struct KfEulerAngles *angles, struct KfMatrix *matrix)
{
    struct KfMatrix temporary;

    func_80014c5c(angles->z, &temporary);
    func_80014b7c(angles->x, matrix);
    MulMatrix(matrix, &temporary);
    func_80014bec(angles->y, &temporary);
    MulMatrix2(&temporary, matrix);
}
