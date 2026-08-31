typedef signed short s16;

struct VecXZ32 {
    int x;
    int unused;
    int z;
};

struct VecXZ16 {
    s16 x;
    s16 z;
};

void func_80014f40(struct VecXZ32 *destination, const struct VecXZ16 *delta)
{
    destination->x += delta->x;
    destination->z += delta->z;
}
