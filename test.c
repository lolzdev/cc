void write(int fd, long int s, int n);

void test()
{
    write(0, "123456789\n", 5);
    write(0, "\n", 1);
}

int main(void)
{
    test();
    return 0;
}
