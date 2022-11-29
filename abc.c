
int main()
{
    int a = 1;
    while (1)
    {
        for (int i = 0; i < 1000000000; i++)
        {
            int b = a + 1;
            for (int j = 0; j < 100000000; j++)
            {
                int c = b + 1;
            }
        }
        a += 1;
        if (a == 1000000000)
        {
            break;
        }
    }
    return a;
}