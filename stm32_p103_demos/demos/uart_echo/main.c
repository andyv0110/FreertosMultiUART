#define USE_STDPERIPH_DRIVER
#include "stm32f10x.h"

int main(void)
{
    uint8_t b;

    init_led();

    init_rs232();
    USART_Cmd(USART1, ENABLE);
    USART_Cmd(USART2, ENABLE);
    USART_Cmd(USART3, ENABLE);
 

    while(1) {
        /* Loop until the USART2 has received a byte. */
        while(USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == RESET);

        /* Capture the received byte and print it out. */
        b = (USART_ReceiveData(USART2) & 0x7F);
        send_byte_uart(USART2,'G');
        send_byte_uart(USART2,'o');
        send_byte_uart(USART2,'t');
        send_byte_uart(USART2,':');
        send_byte_uart(USART2,b);
        send_byte_uart(USART2,'\n');


        send_byte_uart(USART1,'G');
        send_byte_uart(USART1,'o');
        send_byte_uart(USART1,'t');
        send_byte_uart(USART1,':');
        send_byte_uart(USART1,b);
        send_byte_uart(USART1,'\n');

        send_byte_uart(USART3,'G');
        send_byte_uart(USART3,'o');
        send_byte_uart(USART3,'t');
        send_byte_uart(USART3,':');
        send_byte_uart(USART3,b);
        send_byte_uart(USART3,'\n');
    }
}
