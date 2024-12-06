// These are the function prototypes for reading/writing the Mini UART

void uart_init();
void uart_putc(unsigned int c);
char uart_getc();
void uart_puts(char *s);
void uart_puthex(unsigned int value);
