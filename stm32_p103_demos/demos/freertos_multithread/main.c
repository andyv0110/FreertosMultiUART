#define USE_STDPERIPH_DRIVER
#include "stm32f10x.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <string.h>

static void setup_hardware( void );

volatile xQueueHandle serial_str_queue_1 = NULL;
volatile xQueueHandle serial_str_queue_2 = NULL;
volatile xQueueHandle serial_str_queue_3 = NULL;
volatile xSemaphoreHandle serial_tx_wait_sem_1 = NULL;
volatile xSemaphoreHandle serial_tx_wait_sem_2 = NULL;
volatile xSemaphoreHandle serial_tx_wait_sem_3 = NULL;
volatile xQueueHandle serial_rx_queue_1 = NULL;
volatile xQueueHandle serial_rx_queue_2 = NULL;
volatile xQueueHandle serial_rx_queue_3 = NULL;

/* Queue structure used for passing messages. */
typedef struct
{
    char str[100];
} serial_str_msg;

/* Queue structure used for passing characters. */
typedef struct
{
    char ch;
} serial_ch_msg;


/* IRQ handler to handle USART1 interrupts (both transmit and receive
 * interrupts). */
void USART1_IRQHandler(void)
{
    static signed portBASE_TYPE xHigherPriorityTaskWoken;
    serial_ch_msg rx_msg;

    /* If this interrupt is for a transmit... */
    if(USART_GetITStatus(USART1, USART_IT_TXE) != RESET) {
        /* "give" the serial_tx_wait_sem semaphore to notify processes that
         * the buffer has a spot free for the next byte.
         */
        xSemaphoreGiveFromISR(serial_tx_wait_sem_1, &xHigherPriorityTaskWoken);

        /* Disables the transmit interrupt. */
        USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
    /* If this interrupt is for a receive... */
    } else if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        /* Receive the byte from the buffer. */
        rx_msg.ch = USART_ReceiveData(USART1);

        /* Queue the received byte. */
        if(!xQueueSendToBackFromISR(serial_rx_queue_1, &rx_msg, &xHigherPriorityTaskWoken)) {
            /* If there was an error queueing the received byte, freeze. */
            while(1);
        }
    } else {
        /* Only transmit and receive interrupts should be enabled.  If this is
         * another type of interrupt, freeze.
         */
        while(1);
    }

    if(xHigherPriorityTaskWoken) {
        taskYIELD();
    }
}


/* IRQ handler to handle USART2 interrupts (both transmit and receive
 * interrupts). */
void USART2_IRQHandler(void)
{
    static signed portBASE_TYPE xHigherPriorityTaskWoken;
    serial_ch_msg rx_msg;

    /* If this interrupt is for a transmit... */
    if(USART_GetITStatus(USART2, USART_IT_TXE) != RESET) {
        /* "give" the serial_tx_wait_sem semaphore to notify processes that
         * the buffer has a spot free for the next byte.
         */
        xSemaphoreGiveFromISR(serial_tx_wait_sem_2, &xHigherPriorityTaskWoken);

        /* Disables the transmit interrupt. */
        USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
    /* If this interrupt is for a receive... */
    } else if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
        /* Receive the byte from the buffer. */
        rx_msg.ch = USART_ReceiveData(USART2);

        /* Queue the received byte. */
        if(!xQueueSendToBackFromISR(serial_rx_queue_2, &rx_msg, &xHigherPriorityTaskWoken)) {
            /* If there was an error queueing the received byte, freeze. */
            while(1);
        }
    } else {
        /* Only transmit and receive interrupts should be enabled.  If this is
         * another type of interrupt, freeze.
         */
        while(1);
    }

    if(xHigherPriorityTaskWoken) {
        taskYIELD();
    }
}

/* IRQ handler to handle USART3 interrupts (both transmit and receive
 * interrupts). */
void USART3_IRQHandler(void)
{
    static signed portBASE_TYPE xHigherPriorityTaskWoken;
    serial_ch_msg rx_msg;

    /* If this interrupt is for a transmit... */
    if(USART_GetITStatus(USART3, USART_IT_TXE) != RESET) {
        /* "give" the serial_tx_wait_sem semaphore to notify processes that
         * the buffer has a spot free for the next byte.
         */
        xSemaphoreGiveFromISR(serial_tx_wait_sem_3, &xHigherPriorityTaskWoken);

        /* Disables the transmit interrupt. */
        USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
    /* If this interrupt is for a receive... */
    } else if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) {
        /* Receive the byte from the buffer. */
        rx_msg.ch = USART_ReceiveData(USART3);

        /* Queue the received byte. */
        if(!xQueueSendToBackFromISR(serial_rx_queue_3, &rx_msg, &xHigherPriorityTaskWoken)) {
            /* If there was an error queueing the received byte, freeze. */
            while(1);
        }
    } else {
        /* Only transmit and receive interrupts should be enabled.  If this is
         * another type of interrupt, freeze.
         */
        while(1);
    }

    if(xHigherPriorityTaskWoken) {
        taskYIELD();
    }
}

void send_byte_rtos_1(char ch)
{
    /* Wait until the RS232 port can receive another byte (this semaphore is
     * "given" by the RS232 port interrupt when the buffer has room for another
     * byte.
     */
    while(!xSemaphoreTake(serial_tx_wait_sem_1, portMAX_DELAY));

    /* Send the byte and enable the transmit interrupt (it is disabled by the
     * interrupt).
     */
    USART_SendData(USART1, ch);
    USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
}

void send_byte_rtos_2(char ch)
{
    /* Wait until the RS232 port can receive another byte (this semaphore is
     * "given" by the RS232 port interrupt when the buffer has room for another
     * byte.
     */
    while(!xSemaphoreTake(serial_tx_wait_sem_2, portMAX_DELAY));

    /* Send the byte and enable the transmit interrupt (it is disabled by the
     * interrupt).
     */
    USART_SendData(USART2, ch);
    USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
}


void send_byte_rtos_3(char ch)
{
    /* Wait until the RS232 port can receive another byte (this semaphore is
     * "given" by the RS232 port interrupt when the buffer has room for another
     * byte.
     */
    while(!xSemaphoreTake(serial_tx_wait_sem_3, portMAX_DELAY));

    /* Send the byte and enable the transmit interrupt (it is disabled by the
     * interrupt).
     */
    USART_SendData(USART3, ch);
    USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
}

char receive_byte_rtos(void)
{
    serial_ch_msg msg;

    /* Wait for a byte to be queued by the receive interrupt handler. */
    while(!xQueueReceive(serial_rx_queue_3, &msg, portMAX_DELAY));

    return msg.ch;
}

void led_flash_task( void *pvParameters )
{
    while(1) {
        /* Toggle the LED. */
        GPIOC->ODR = GPIOC->ODR ^ 0x00001000;

        /* Wait one second. */
        vTaskDelay(100);
    }
}

void rs232_xmit_msg_task_1( void *pvParameters )
{
    serial_str_msg msg;
    int curr_char;

    while(1) {
        /* Read from the queue.  Keep trying until a message is received.  This
         * will block for a period of time (specified by portMAX_DELAY). */
        while(!xQueueReceive(serial_str_queue_1, &msg, portMAX_DELAY));

        /* Write each character of the message to the RS232 port. */
        curr_char = 0;
        while(msg.str[curr_char] != '\0') {
            send_byte_rtos_1(msg.str[curr_char]);
            curr_char++;
        }
    }
}

void rs232_xmit_msg_task_2( void *pvParameters )
{
    serial_str_msg msg;
    int curr_char;

    while(1) {
        /* Read from the queue.  Keep trying until a message is received.  This
         * will block for a period of time (specified by portMAX_DELAY). */
        while(!xQueueReceive(serial_str_queue_2, &msg, portMAX_DELAY));

        /* Write each character of the message to the RS232 port. */
        curr_char = 0;
        while(msg.str[curr_char] != '\0') {
            send_byte_rtos_2(msg.str[curr_char]);
            curr_char++;
        }
    }
}


void rs232_xmit_msg_task_3( void *pvParameters )
{
    serial_str_msg msg;
    int curr_char;

    while(1) {
        /* Read from the queue.  Keep trying until a message is received.  This
         * will block for a period of time (specified by portMAX_DELAY). */
        while(!xQueueReceive(serial_str_queue_3, &msg, portMAX_DELAY));

        /* Write each character of the message to the RS232 port. */
        curr_char = 0;
        while(msg.str[curr_char] != '\0') {
            send_byte_rtos_3(msg.str[curr_char]);
            curr_char++;
        }
    }
}

/* Repeatedly queues a string to be sent to the RS232.
 *   delay - the time to wait between sending messages.  A delay of 1 means
 *           wait 1/100th of a second.
 */
void queue_str_task(const char *str, int delay)
{
    serial_str_msg msg1;
    serial_str_msg msg2;
    serial_str_msg msg3;
    /* Prepare the message to be queued. */
    strcpy(msg1.str, "USART1\n");
    strcpy(msg2.str, str);
    strcpy(msg3.str, "USART3\n");

    while(1) {
        /* Post the message.  Keep on trying until it is successful. */
        while(!xQueueSendToBack(serial_str_queue_1, &msg1, portMAX_DELAY));
	while(!xQueueSendToBack(serial_str_queue_2, &msg2, portMAX_DELAY));
	//while(!xQueueSendToBack(serial_str_queue_3, &msg3, portMAX_DELAY));

        /* Wait. */
        vTaskDelay(delay);
    }
}

void queue_str_task1( void *pvParameters )
{
    queue_str_task("Hello 1\n", 500);
}

void queue_str_task2( void *pvParameters )
{
    queue_str_task("Hello 2\n", 1000);
}

void serial_readwrite_task( void *pvParameters )
{
    serial_str_msg msg;
    char ch;
    int curr_char;
    int done;

    /* Prepare the response message to be queued. */
    strcpy(msg.str, "Got:");

    while(1) {
        curr_char = 4;
        done = 0;
        do {
            /* Receive a byte from the RS232 port (this call will block). */
            ch = receive_byte_rtos();

            /* If the byte is an end-of-line type character, then finish the
             * string and indicate we are done.
             */
            if((ch == '\r') || (ch == '\n')) {
		msg.str[curr_char] = curr_char+'1';                
		msg.str[curr_char+1] = '\n';
                msg.str[curr_char+2] = '\0';
                done = -1;
            /* Otherwise, add the character to the response string. */
            } else {
                msg.str[curr_char++] = ch;
            }
        } while(!done);

        /* Once we are done building the response string, queue the response to
         * be sent to the RS232 port.
         */
        while(!xQueueSendToBack(serial_str_queue_3, &msg, portMAX_DELAY));
    }
}

int main(void)
{
    init_led();

    init_button();
    enable_button_interrupts();

    init_rs232();
    enable_rs232_interrupts();
    enable_rs232();


    /* Create the queue to hold messages to be written to the RS232. */
    serial_str_queue_1 = xQueueCreate( 10, sizeof( serial_str_msg ) );
    vSemaphoreCreateBinary(serial_tx_wait_sem_1);
    serial_rx_queue_1 = xQueueCreate( 1, sizeof( serial_ch_msg ) );
    /* Create the queue to hold messages to be written to the RS232. */
    serial_str_queue_2 = xQueueCreate( 10, sizeof( serial_str_msg ) );
    vSemaphoreCreateBinary(serial_tx_wait_sem_2);
    serial_rx_queue_2 = xQueueCreate( 1, sizeof( serial_ch_msg ) );
   /* Create the queue to hold messages to be written to the RS232. */
    serial_str_queue_3 = xQueueCreate( 10, sizeof( serial_str_msg ) );
    vSemaphoreCreateBinary(serial_tx_wait_sem_3);
    serial_rx_queue_3 = xQueueCreate( 1, sizeof( serial_ch_msg ) );

    /* Create a task to flash the LED. */
    xTaskCreate( led_flash_task, ( signed portCHAR * ) "LED Flash", 256/* stack size */, NULL, tskIDLE_PRIORITY + 5, NULL );

    /* Create tasks to queue a string to be written to the RS232 port. */
    xTaskCreate( queue_str_task1, ( signed portCHAR * ) "Serial Write 1", 256 /* stack size */, NULL, tskIDLE_PRIORITY + 10, NULL );
    xTaskCreate( queue_str_task2, ( signed portCHAR * ) "Serial Write 2", 256 /* stack size */, NULL, tskIDLE_PRIORITY + 10, NULL );

    /* Create a task to write messages from the queue to the RS232 port. */
    xTaskCreate(rs232_xmit_msg_task_1, ( signed portCHAR * ) "Serial Xmit Str 1", 256 /* stack size */, NULL, tskIDLE_PRIORITY + 2, NULL );
    /* Create a task to write messages from the queue to the RS232 port. */
    xTaskCreate(rs232_xmit_msg_task_2, ( signed portCHAR * ) "Serial Xmit Str 1", 256 /* stack size */, NULL, tskIDLE_PRIORITY + 2, NULL );
    /* Create a task to write messages from the queue to the RS232 port. */
    xTaskCreate(rs232_xmit_msg_task_3, ( signed portCHAR * ) "Serial Xmit Str 1", 256 /* stack size */, NULL, tskIDLE_PRIORITY + 2, NULL );

    /* Create a task to receive characters from the RS232 port and echo them back to the RS232 port. */
    xTaskCreate(serial_readwrite_task, ( signed portCHAR * ) "Serial Read/Write", 256 /* stack size */, NULL, tskIDLE_PRIORITY + 10, NULL );

    /* Start running the tasks. */
    vTaskStartScheduler();

    return 0;
}

void vApplicationTickHook( void )
{
}
