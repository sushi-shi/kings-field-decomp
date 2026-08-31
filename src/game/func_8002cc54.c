typedef signed short s16;

struct Fields8002cc54 {
    unsigned char unknown[44];
    s16 first;
    s16 second;
    s16 third;
};

void func_8002cc54(
    struct Fields8002cc54 *object,
    s16 first,
    s16 second,
    s16 third)
{
    object->first = first;
    object->second = second;
    object->third = third;
}
