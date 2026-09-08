/* Synthetic compiler controls; these are not reconstructed game source. */
extern int observe(int);
extern void consume_words(int *, int);
extern void consume_bytes(unsigned char *);

int crossing_absent(int value)
{
    return observe(value + 7);
}

int crossing_present(int value)
{
    int saved = value + 7;
    return observe(value) + saved;
}

struct TraceObject {
    int words[4];
    unsigned char enabled, visible;
};
extern struct TraceObject object;
extern struct TraceObject objects[2];
extern void consume_object(struct TraceObject *);

void member_folded(void)
{
    struct TraceObject *first, *second;
    objects[0].enabled = objects[1].enabled = 1;
    first = &objects[0];
    second = &objects[1];
    first->visible = 0;
    second->visible = 0;
    consume_object(first);
    consume_object(second);
    first->visible = 1;
    second->visible = 1;
}

void member_retained(struct TraceObject *first, struct TraceObject *second)
{
    first->enabled = second->enabled = 1;
    first->visible = 0;
    second->visible = 0;
    consume_object(first);
    consume_object(second);
    first->visible = 1;
    second->visible = 1;
}

void member_root_and_offset(void)
{
    object.words[0] = 1;
    object.words[1] = 2;
    consume_object(&object);
    object.words[0] = 1;
    object.words[1] = 2;
    consume_object(&object);
}

void member_absolute(void)
{
    object.enabled = 1;
    consume_bytes(&object.enabled);
    object.visible = 1;
}

void member_relative(struct TraceObject *p)
{
    p->enabled = 1;
    consume_bytes(&p->enabled);
    p->visible = 1;
}

int tails_equal(int condition, int value)
{
    if (condition) {
        observe(value);
        object.words[0] = 1;
        object.words[1] = 0;
    } else {
        observe(value + 1);
        object.words[0] = 1;
        object.words[1] = 0;
    }
    return object.words[0];
}

int tails_distinct(int condition, int value)
{
    if (condition) {
        observe(value);
        object.words[0] = 1;
        object.words[1] = 0;
    } else {
        observe(value + 1);
        object.words[0] = 2;
        object.words[1] = 0;
    }
    return object.words[0];
}

int stack_small(void)
{
    int values[3] = {1, 2, 3};
    consume_words(values, 3);
    return values[1];
}

int stack_large(void)
{
    int values[9] = {1, 2, 3};
    consume_words(values, 9);
    return values[1];
}

void stack_reuse(void)
{
    {
        int values[3] = {1, 2, 3};
        consume_words(values, 3);
    }
    {
        int values[3] = {4, 5, 6};
        consume_words(values, 3);
    }
}

int cse_equivalent(int a, int b)
{
    return (a + b) * (a + b);
}

int schedule_chain(int *p)
{
    int a = p[0];
    return p[a] + 3;
}

int schedule_independent(int *p)
{
    int a = p[0];
    int b = p[1];
    return a + b + 3;
}

int reload_pressure(int *p, int condition)
{
    int a = p[0], b = p[1], c = p[2], d = p[3];
    int e = p[4], f = p[5], g = p[6], h = p[7];
    int i = p[8], j = p[9], k = p[10], l = p[11];
    if (condition) {
        observe(a + c + e + g + i + k);
        object.words[0] = 1;
    } else {
        observe(b + d + f + h + j + l);
        object.words[0] = 2;
    }
    return a + b + c + d + e + f + g + h + i + j + k + l;
}

extern int observe4(int, int, int, int);
extern long long observe_wide4(int, int, int, int);

int phase_one_spill(int *p, int count)
{
    int v0 = p[0], v1 = p[1], v2 = p[2], v3 = p[3], v4 = p[4], v5 = p[5], v6 = p[6], v7 = p[7];
    short phase = 0;
    int blend = 0;

    while (count--) {
        switch (phase) {
        case 0:
            if (blend <= 4096) {
                blend += (int)(observe_wide4(v0, v1, v2, v3) >> 32);
            } else {
                phase = 1;
                blend = 0;
            }
            break;
        case 1:
            if (blend <= 4096) {
                blend += (int)(observe_wide4(v4, v5, v6, v7) >> 32);
            } else {
                phase = 2;
                blend = 0;
            }
            break;
        }
        v0 += p[0];
        v1 += p[1];
        v2 += p[2];
        v3 += p[3];
        v4 += p[4];
        v5 += p[5];
        v6 += p[6];
        v7 += p[7];
    }
    return v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7;
}

long long phase_two_spills(int *p, int count)
{
    int v0 = p[0], v1 = p[1], v2 = p[2], v3 = p[3], v4 = p[4], v5 = p[5], v6 = p[6], v7 = p[7];
    short phase = 0;
    int blend = 0;

    while (count--) {
        switch (phase) {
        case 0:
            if (blend <= 4096) {
                blend += observe4(v0, v1, v2, v3);
            } else {
                phase = 1;
                blend = 0;
            }
            break;
        case 1:
            if (blend <= 4096) {
                blend += observe4(v4, v5, v6, v7);
            } else {
                phase = 2;
                blend = 0;
            }
            break;
        }
        v0 += p[0];
        v1 += p[1];
        v2 += p[2];
        v3 += p[3];
        v4 += p[4];
        v5 += p[5];
        v6 += p[6];
        v7 += p[7];
    }
    return (long long)v0 * v1 + v2 + v3 + v4 + v5 + v6 + v7;
}

long long constant_competition(int *p, int count)
{
    int v0 = 1, v1 = 2, v2 = p[2], v3 = p[3], v4 = p[4], v5 = p[5], v6 = p[6], v7 = p[7], v8 = p[8], v9 = p[9];
    short phase = 0;
    int blend = 0;

    while (count--) {
        switch (phase) {
        case 0:
            if (blend <= 4096) {
                blend += observe4(v0, v1, v2, v3);
            } else {
                phase = 1;
                blend = 0;
            }
            break;
        case 1:
            if (blend <= 4096) {
                blend += observe4(v4, v5, v6, v7);
            } else {
                phase = 2;
                blend = 0;
            }
            break;
        }
        v0 += p[0];
        v1 += p[1];
        v2 += p[2];
        v3 += p[3];
        v4 += p[4];
        v5 += p[5];
        v6 += p[6];
        v7 += p[7];
        v8 += p[8];
        v9 += p[9];
    }
    return (long long)v0 * v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
}

/* Spilled scalar homes follow the existing declaration pair. */
int spill_order_declared(int *input)
{
    int value_0;
    int value_1;
    int value_2;
    int value_3;
    int value_4;
    int value_5;
    int value_6;
    int value_7;
    int value_8;
    int value_9;
    int value_10;
    int value_11;

    value_0 = input[0];
    value_1 = input[1];
    value_2 = input[2];
    value_3 = input[3];
    value_4 = input[4];
    value_5 = input[5];
    value_6 = input[6];
    value_7 = input[7];
    value_8 = input[8];
    value_9 = input[9];
    value_10 = input[10];
    value_11 = input[11];
    observe(0);
    return value_0 + value_1 + value_2 + value_3 + value_4 + value_5
        + value_6 + value_7 + value_8 + value_9 + value_10 + value_11;
}

int spill_order_reversed(int *input)
{
    int value_0;
    int value_1;
    int value_2;
    int value_3;
    int value_4;
    int value_5;
    int value_6;
    int value_7;
    int value_8;
    int value_10;
    int value_9;
    int value_11;

    value_0 = input[0];
    value_1 = input[1];
    value_2 = input[2];
    value_3 = input[3];
    value_4 = input[4];
    value_5 = input[5];
    value_6 = input[6];
    value_7 = input[7];
    value_8 = input[8];
    value_9 = input[9];
    value_10 = input[10];
    value_11 = input[11];
    observe(0);
    return value_0 + value_1 + value_2 + value_3 + value_4 + value_5
        + value_6 + value_7 + value_8 + value_9 + value_10 + value_11;
}
