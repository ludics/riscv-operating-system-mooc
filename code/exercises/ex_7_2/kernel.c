extern void uart_init(void);
extern void uart_puts(char *s);
extern void uart_gets(char *s, int size);
extern void uart_gets_echo(char *s, int size);
extern void uart_getc_echo(void);

void start_kernel(void)
{
	uart_init();
	uart_puts("Hello, RVOS!\n");
	uart_puts("Type something and press Enter:\n");
	uart_getc_echo();
	while (1) {}; // stop here!
}

