void repeat_store_word(int *destination, int count, int value)
{

    while (count-- != 0) {
        *destination = value;
    }
}
