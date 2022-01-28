

void fLib_memcpy(unsigned char *s, unsigned char *ct, unsigned int n)
{
    while (n--)
        *s++ = *ct++;
}

void fLib_memset(char *s, int c, int n)
{
    int i = 0;

    while (n--)
    {
        s[i++] = c;
    }
}
