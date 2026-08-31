typedef signed short s16;

struct TripleAxes16 {
    s16 x0;
    s16 x1;
    s16 x2;
    s16 y0;
    s16 y1;
    s16 y2;
    s16 z0;
    s16 z1;
    s16 z2;
};

void func_80033de8(s16 x, s16 y, s16 z, struct TripleAxes16 *output)
{
    output->x2 = x;
    output->x1 = x;
    output->x0 = x;
    output->y2 = y;
    output->y1 = y;
    output->y0 = y;
    output->z2 = z;
    output->z1 = z;
    output->z0 = z;
}
