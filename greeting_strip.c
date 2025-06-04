__attribute((naked))
void _start(void)
{
    asm (
        ".align	2\n"
        ".LC0:\n"
        ".ascii	\"Hello World!\\012\\000\"\n"
        ".align	2\n"
        "start:\n"
        "ldr	r1, .L4\n"
        "push	{r7, r9, lr}\n"
        "mov	r0, #1\n"
        "mov	r7, #4\n"
        ".LPIC0:\n"
        "add	r1, pc, r1\n"
        "mov	r2, #13\n"
        "swi     #0\n"
        "pop	{r7, r9, pc}\n"
        ".L5:\n"
        ".align	2\n"
        ".L4:\n"
        ".word	.LC0-(.LPIC0+8)\n"
        );
};
