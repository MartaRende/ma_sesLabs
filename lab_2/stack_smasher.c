int main(void)
{
    char buf[16];
    /* overflow buf to smash the stack */
    for (int i = 0; i < 30; i++) {
      buf[i] = 34;
    }
    return 0;
}
