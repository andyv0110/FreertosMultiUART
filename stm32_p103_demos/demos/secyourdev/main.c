#define USE_STDPERIPH_DRIVER
#include "stm32f10x.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <string.h>
#include <stdlib.h>

/* defines pf the project*/
#define H_EAU_MAX 100
#define H_EAU_MIN 0
#define H_EAU_STEP 10
#define H_EAU_UP 0
#define H_EAU_DOWN 1
#define TEMPO_SEND 0


/* Chaine de caractère pour effacement de l ecran */
char g_CLEAR_SCREEN[] ="\033[2J\033[H";

/* Chaine de caractère du menu principal */
char g_M0_L1[] = "Bienvenue sur la console de gestion du systeme\r\n";
char g_M0_L2[] = "MENU  PRINCIPAL\r\n";
char g_M0_L3[] = "10 : Activer le systeme\r\n";
char g_M0_L4[] = "11 : Arreter le systeme\r\n";
char g_M0_L5[] = "20 : Changer la hauteur max\r\n";
char g_M0_L6[] = "21 : Changer la temperature max\r\n";
char g_M0_L7[] = "22 : Changer la vitesse max\r\n";
char g_M0_L8[] = "30 : Changer la temperature \r\n";
char g_M0_L9[] = "31 : Changer la vitesse \r\n";
char g_M0_L10[] = "40 : Allumer lumiere\r\n";
char g_M0_L11[] = "41 : Eteindre lumiere\r\n";
char g_M0_L12[] = "50 : Allumer le Haut Parleur\r\n";
char g_M0_L13[] = "51 : Eteindre le Haut Parleur\r\n";
char g_M0_L14[] = "60 : Afficher Etat Valeur MAX et courantes\r\n";
char g_M0_L15[] = "Saisissez votre choix : ";



/* Chaine de caractère du menu affichage des valeurs */
char * g_M60_L1 = g_M0_L1;
char g_M60_L2[] = "MENU : Principal / Valeurs max et courantes\r\n";
char g_M60_L3[] = "H_EAU_MAX : ";
char g_M60_L4[] = "T_EAU_MAX :";
char g_M60_L5[] = "V_VENT_MAX :";
char g_M60_L6[] = "H_EAU :";
char g_M60_L7[] = "T_EAU :";
char g_M60_L8[] = "V_VENT :";
char g_M60_L9[] = "LUM :";
char g_M60_L10[] = "HP :";
char g_M60_L11[] = "SYS :";
char g_M60_L12[] = "Appuyez sur q et entrée pour quitter.";

/* Chaine de caractère du menu Valeur H_EAU_MAX */
char * g_M20_L1 = g_M0_L1;
char g_M20_L2[] = "MENU : Principal / Changer la Hauteur max\r\n";
char g_M20_L3[] = "Veuillez saisir la hauteur max (de 60 a 100) : ";


/* Chaine de caractère du menu Valeur T_EAU_MAX */
char * g_M21_L1 = g_M0_L1;
char g_M21_L2[] = "MENU : Principal / Changer la Temperature max\r\n";
char g_M21_L3[] = "Veuillez saisir la température max  (de 160 a 200) : ";

/* Chaine de caractère du menu Valeur V_VENT_MAX */
char * g_M22_L1 = g_M0_L1;
char g_M22_L2[] = "MENU : Principal /Changer la Vitesse max\r\n";
char g_M22_L3[] = "Veuillez saisir la vitesse max (de 40 a 50) : ";

/* Chaine de caractère du menu Valeur T_EAU_MAX */
char * g_M30_L1 = g_M0_L1;
char g_M30_L2[] = "MENU : Principal / Changer la Temperature\r\n";
char g_M30_L3[] = "Veuillez saisir la température : ";

/* Chaine de caractère du menu Valeur V_VENT_MAX */
char * g_M31_L1 = g_M0_L1;
char g_M31_L2[] = "MENU : Principal /Changer la Vitesse \r\n";
char g_M31_L3[] = "Veuillez saisir la vitesse  : ";

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

/*global variable*/
unsigned char g_H_EAU = 10;
unsigned char g_H_EAU_MAX = 100;
unsigned char g_T_EAU = 20;
unsigned char g_T_EAU_MAX = 200;
unsigned char g_V_VENT = 0;
unsigned char g_V_VENT_MAX = 50;

enum e_Menu_Current{
    E_MENU_PRINCIPAL = 0, 
    E_MENU_SYS_ON = 10, 
    E_MENU_SYS_OFF = 11, 
    E_MENU_CHANGE_H_EAU_MAX = 20, 
    E_MENU_CHANGE_T_EAU_MAX = 21, 
    E_MENU_CHANGE_V_VENT_MAX = 22, 
    E_MENU_CHANGE_T_EAU = 30, 
    E_MENU_CHANGE_V_VENT = 31,
    E_MENU_LIGHT_ON = 40, 
    E_MENU_LIGHT_OFF = 41,
    E_MENU_HP_ON = 50, 
    E_MENU_HP_OFF = 51,
    E_MENU_SHOW_VALUE = 60, 
}; 

enum e_Menu_Current g_e_Menu_Current = E_MENU_PRINCIPAL;

enum e_State {
    E_STATE_OFF = 0, 
    E_STATE_ON = 1, 
};

enum e_State g_e_State_HP = E_STATE_OFF;
enum e_State g_e_State_Light = E_STATE_OFF;
enum e_State g_e_State_System = E_STATE_OFF;

/* Queue structure used for passing messages. */
typedef struct
{
    char str[50];
} serial_str_msg;

/* Queue structure used for passing characters. */
typedef struct
{
    char ch;
} serial_ch_msg;


/* Queue structure used for passing commande */
typedef struct
{
    unsigned char id;
    unsigned char param;
} t_s_cmd;


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


void send_byte_rtos(USART_TypeDef* USARTx,char ch)
{
    /* Wait until the RS232 port can receive another byte (this semaphore is
     * "given" by the RS232 port interrupt when the buffer has room for another
     * byte.
     */
    volatile xSemaphoreHandle l_serial_tx_wait_sem = NULL;
    if (USARTx == USART1) {
	l_serial_tx_wait_sem = serial_tx_wait_sem_1;
    }
    else if (USARTx == USART2) {
	l_serial_tx_wait_sem = serial_tx_wait_sem_2;
    }
    else if (USARTx == USART3) {
	l_serial_tx_wait_sem = serial_tx_wait_sem_3;
    }
    else {
       while (1);   
    }
    while(!xSemaphoreTake(l_serial_tx_wait_sem, portMAX_DELAY));

    /* Send the byte and enable the transmit interrupt (it is disabled by the
     * interrupt).
     */
    USART_SendData(USARTx, ch);
    USART_ITConfig(USARTx, USART_IT_TXE, ENABLE);
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

void rs232_xmit_msg_task_1(void *pvParameters )
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
            send_byte_rtos(USART1, msg.str[curr_char]);
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
            send_byte_rtos(USART2,msg.str[curr_char]);
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
            send_byte_rtos(USART3,msg.str[curr_char]);
            curr_char++;
        }
    }
}

/* Repeatedly queues a string to be sent to the RS232.
 *   delay - the time to wait between sending messages.  A delay of 1 means
 *           wait 1/100th of a second.
 */
void queue_str_to_send(USART_TypeDef* USARTx,const char *str, int delay)
{
    serial_str_msg msg;
    volatile xQueueHandle l_serial_str_queue = NULL;

    if (USARTx == USART1) {
	l_serial_str_queue = serial_str_queue_1;
    }
    else if (USARTx == USART2) {
	l_serial_str_queue = serial_str_queue_2;
    }
    else if (USARTx == USART3) {
	l_serial_str_queue = serial_str_queue_3;
    }
    else {
       while (1);   
    }

    /* Prepare the message to be queued. */
    strcpy(msg.str, str);
 
   // while(0) {
        /* Post the message.  Keep on trying until it is successful. */
        while(!xQueueSendToBack(l_serial_str_queue, &msg, portMAX_DELAY));

        /* Wait. */
        vTaskDelay(delay);
  //  }
}

void queue_str_task1( void *pvParameters )
{
    unsigned char l_way = H_EAU_UP;
    char str[10];
    while (1) {
	if (g_e_State_System == E_STATE_ON) {
		/* Step 1 : Identify if the way should change */
		if (l_way == H_EAU_UP) {
		    if ((g_H_EAU + H_EAU_STEP) > g_H_EAU_MAX) {
			l_way = H_EAU_DOWN;
			queue_str_to_send(USART1, "Down\r\n", 10);
		    }
		    else {
			/* Si le niveau d'eau ne depasse pas le max, le sens ne change pas*/
		    }
		}
		else if (l_way == H_EAU_DOWN) {
		    if ((g_H_EAU - H_EAU_STEP) < H_EAU_MIN) {
			l_way = H_EAU_UP;
			queue_str_to_send(USART1, "Up\r\n", 10);
		    }
		    else {
			/* Si le niveau d'eau ne depasse pas le max, le sens ne change pas*/
		    }
		}
		else {
		    /* Erreur */
		    //while (1);
		}

		/* Step 2 : Update de value of the height pf the water */
		if (l_way == H_EAU_UP) {
		    g_H_EAU += H_EAU_STEP;
	    	    queue_str_to_send(USART1, "CMD_H_EAU_++\r\n", 200);
		}
		else if (l_way == H_EAU_DOWN) {
		    g_H_EAU -= H_EAU_STEP;
	    	    queue_str_to_send(USART1, "CMD_H_EAU_--\r\n", 200);
		}
		else {
		   /*Erreur*/
		   //while (1);
		}

		/* Step 3 : Display value of g_H_EAU*/
		str[0] = hex_to_char(g_H_EAU/10);
		str[1] = '\n';
		str[2] = '\r';
		str[3] = '\0';
		queue_str_to_send(USART1, str, 10);
	}
    }
}

void queue_str_task2( void *pvParameters )
{
   while (1) { 
      // queue_str_to_send(USART2,"\033[2J", 10);
      // queue_str_to_send(USART2,"\033[H", 10);
       queue_str_to_send(USART2,"USART2\r\n", 400);
    }
}

unsigned char strtouc(char * str) {
	unsigned char l_ucVar = 0;
	char l_char[4];
	l_char[0]= strlen(str) + '0'-3;
	l_char[1]='\r';
	l_char[2]='\n';
	l_char[3]='\0';
	while(!xQueueSendToBack(serial_str_queue_3, l_char, portMAX_DELAY));
	/* Wait. */
        vTaskDelay(100);

	if (strlen(str) == 4) {
		l_ucVar = str[0]-'0';
	}
	else if (strlen(str) == 5) {
		l_ucVar = (str[0]-'0')*10 +str[1]-'0';
	}
	else if (strlen(str) == 6) {
		l_ucVar = (str[0]-'0')*100 +(str[1]-'0')*10 +str[2]-'0';
	}	
	return l_ucVar;
}


void uctostr(unsigned char data, char str[]) {
	str[0] = data/100 +'0'; 
	str[1] = (data%100)/10 +'0'; 
	str[2] = (data%10) +'0'; 
	str[3] = '\r'; 
	str[4] = '\n'; 
	str[5] = '\0'; 
}


void Handle_Cmd(serial_str_msg * msg) {
        /* Once we are done building the response string, queue the response to
         * be sent to the RS232 port.
         */
	t_s_cmd ls_cmd;
	char l_tcParam[6];
	
        while(!xQueueSendToBack(serial_str_queue_3, msg, portMAX_DELAY));
	if (g_e_Menu_Current == E_MENU_PRINCIPAL) {
		if (strncmp(msg->str,"10",2) == 0) {
			while(!xQueueSendToBack(serial_str_queue_3, "Command System ON received", portMAX_DELAY));	
			g_e_State_System = E_STATE_ON;
		}
		else if (strncmp(msg->str,"11",2) == 0) {
			while(!xQueueSendToBack(serial_str_queue_3, "Command System OFF received", portMAX_DELAY));
			g_e_State_System = E_STATE_OFF;	
		}
		else if (strncmp(msg->str,"20",2) == 0) {
			while(!xQueueSendToBack(serial_str_queue_3, "Command Change Height max received", portMAX_DELAY));
		    	g_e_Menu_Current = E_MENU_CHANGE_H_EAU_MAX; 
		}
		else if (strncmp(msg->str,"21",2) == 0) {
			while(!xQueueSendToBack(serial_str_queue_3, "Command Change Temperature max received", portMAX_DELAY));	
			g_e_Menu_Current = E_MENU_CHANGE_T_EAU_MAX; 
		}
		else if (strncmp(msg->str,"22",2) == 0) {
			while(!xQueueSendToBack(serial_str_queue_3, "Command Change Speed max received", portMAX_DELAY));	
			g_e_Menu_Current = E_MENU_CHANGE_V_VENT_MAX;
		}
		else if (strncmp(msg->str,"30",2) == 0) {
			while(!xQueueSendToBack(serial_str_queue_3, "Command Change Temperature received", portMAX_DELAY));
			g_e_Menu_Current = E_MENU_CHANGE_T_EAU; 
		}
		else if (strncmp(msg->str,"31",2) == 0) {
			while(!xQueueSendToBack(serial_str_queue_3, "Command Change Speed received", portMAX_DELAY));
			g_e_Menu_Current = E_MENU_CHANGE_V_VENT;		
		}
		else if (strncmp(msg->str,"40",2) == 0) {
			while(!xQueueSendToBack(serial_str_queue_3, "Command Light ON received", portMAX_DELAY));
			g_e_State_Light = E_STATE_ON;		
		}
		else if (strncmp(msg->str,"41",2) == 0) {
			while(!xQueueSendToBack(serial_str_queue_3, "Command Light OFF received", portMAX_DELAY));
			g_e_State_Light = E_STATE_OFF;		
		}
		else if (strncmp(msg->str,"50",2) == 0) {
			while(!xQueueSendToBack(serial_str_queue_3, "Command HP ON received", portMAX_DELAY));	
			g_e_State_HP = E_STATE_ON;
		}
		else if (strncmp(msg->str,"51",2) == 0) {
			while(!xQueueSendToBack(serial_str_queue_3, "Command HP OFF received", portMAX_DELAY));	
			g_e_State_HP = E_STATE_OFF;
		}
		else if (strncmp(msg->str,"60",2) == 0) {
			while(!xQueueSendToBack(serial_str_queue_3, "Command Value received", portMAX_DELAY));	
			g_e_Menu_Current = E_MENU_SHOW_VALUE;
		}
		else if (strncmp(msg->str,"q",1) == 0) {
			while(!xQueueSendToBack(serial_str_queue_3, "Command Value received", portMAX_DELAY));	
			g_e_Menu_Current = E_MENU_PRINCIPAL;
		}
		else {
			while(!xQueueSendToBack(serial_str_queue_3, "Error : Invalid command\r\n", portMAX_DELAY));
		}
	}
	else if (g_e_Menu_Current == E_MENU_CHANGE_H_EAU_MAX) {
		g_H_EAU_MAX = strtouc(msg->str);
		g_e_Menu_Current = E_MENU_PRINCIPAL;
	}
	else if (g_e_Menu_Current == E_MENU_CHANGE_T_EAU_MAX) {
		g_T_EAU_MAX = strtouc(msg->str);
		g_e_Menu_Current = E_MENU_PRINCIPAL;
	}
	else if (g_e_Menu_Current == E_MENU_CHANGE_V_VENT_MAX) {
		g_V_VENT_MAX = strtouc(msg->str);
		g_e_Menu_Current = E_MENU_PRINCIPAL;
	}
	else if (g_e_Menu_Current == E_MENU_CHANGE_T_EAU) {
		g_T_EAU = strtouc(msg->str);
		uctostr(g_T_EAU,l_tcParam);
		queue_str_to_send(USART1, l_tcParam, 10);
		g_e_Menu_Current = E_MENU_PRINCIPAL;
	}
	else if (g_e_Menu_Current == E_MENU_CHANGE_V_VENT) {
		g_V_VENT = strtouc(msg->str);
		uctostr(g_V_VENT,l_tcParam);
		queue_str_to_send(USART2, l_tcParam, 10);
		g_e_Menu_Current = E_MENU_PRINCIPAL;
	}
	else {
		g_e_Menu_Current = E_MENU_PRINCIPAL;
	}
}

void Print_Menu_Main(void){
        queue_str_to_send(USART3,g_CLEAR_SCREEN,TEMPO_SEND);
        queue_str_to_send(USART3,g_M0_L1, TEMPO_SEND);
        queue_str_to_send(USART3,g_M0_L2, TEMPO_SEND);
        queue_str_to_send(USART3,g_M0_L3, TEMPO_SEND);
        queue_str_to_send(USART3,g_M0_L4, TEMPO_SEND);
        queue_str_to_send(USART3,g_M0_L5, TEMPO_SEND);
        queue_str_to_send(USART3,g_M0_L6, TEMPO_SEND);
        queue_str_to_send(USART3,g_M0_L7, TEMPO_SEND);
        queue_str_to_send(USART3,g_M0_L8, TEMPO_SEND);
        queue_str_to_send(USART3,g_M0_L9, TEMPO_SEND);
        queue_str_to_send(USART3,g_M0_L10, TEMPO_SEND);
        queue_str_to_send(USART3,g_M0_L11, TEMPO_SEND);
        queue_str_to_send(USART3,g_M0_L12, TEMPO_SEND);
        queue_str_to_send(USART3,g_M0_L13, TEMPO_SEND);
        queue_str_to_send(USART3,g_M0_L14, TEMPO_SEND);
        queue_str_to_send(USART3,g_M0_L15, TEMPO_SEND);
}


/* Functions for sending numbers through the UART */
unsigned char string_to_uchar(char p_t_char[], unsigned char size)
{
    return (p_t_char[1]-'0')*16 + (p_t_char[1]-'0');
}





void Print_Menu_Value(void){
	char var[6];
        queue_str_to_send(USART3,g_CLEAR_SCREEN,TEMPO_SEND);
        queue_str_to_send(USART3,g_M60_L1, TEMPO_SEND);
        queue_str_to_send(USART3,g_M60_L2, TEMPO_SEND); 
	/* H_EAU_MAX */
        queue_str_to_send(USART3,g_M60_L3, TEMPO_SEND);
	var[0] = '0'+g_H_EAU_MAX/100;
	var[1] = '0'+(g_H_EAU_MAX%100)/10;
	var[2] = '0'+g_H_EAU_MAX%10;
	var[3] = '\r';
	var[4] = '\n';
	var[5] = '\0';
	queue_str_to_send(USART3, var, TEMPO_SEND);
	/* T_EAU_MAX */
        queue_str_to_send(USART3,g_M60_L4, TEMPO_SEND);
	var[0] = '0'+g_T_EAU_MAX/100;
	var[1] = '0'+(g_T_EAU_MAX%100)/10;
	var[2] = '0'+g_T_EAU_MAX%10;
	var[3] = '\r';
	var[4] = '\n';
	var[5] = '\0';
	queue_str_to_send(USART3, var, TEMPO_SEND);
	/* V_VENT_MAX */
        queue_str_to_send(USART3,g_M60_L5, TEMPO_SEND);
	var[0] = '0'+g_V_VENT_MAX/100;
	var[1] = '0'+(g_V_VENT_MAX%100)/10;
	var[2] = '0'+g_V_VENT_MAX%10;
	var[3] = '\r';
	var[4] = '\n';
	var[5] = '\0';
	queue_str_to_send(USART3, var, TEMPO_SEND);
	/* H_EAU */
        queue_str_to_send(USART3,g_M60_L6, TEMPO_SEND);
	var[0] = '0'+g_H_EAU/100;
	var[1] = '0'+(g_H_EAU%100)/10;
	var[2] = '0'+g_H_EAU%10;
	var[3] = '\r';
	var[4] = '\n';
	var[5] = '\0';
	queue_str_to_send(USART3, var, TEMPO_SEND);
	/* T_EAU */
        queue_str_to_send(USART3,g_M60_L7, TEMPO_SEND);
	var[0] = '0'+g_T_EAU/100;
	var[1] = '0'+(g_T_EAU%100)/10;
	var[2] = '0'+g_T_EAU%10;
	var[3] = '\r';
	var[4] = '\n';
	var[5] = '\0';
	queue_str_to_send(USART3, var, TEMPO_SEND);
	/* V_VENT */
        queue_str_to_send(USART3,g_M60_L8, TEMPO_SEND);
	var[0] = '0'+g_V_VENT/100;
	var[1] = '0'+(g_V_VENT%100)/10;
	var[2] = '0'+g_V_VENT%10;
	var[3] = '\r';
	var[4] = '\n';
	var[5] = '\0';
	queue_str_to_send(USART3, var, TEMPO_SEND);
	/* Lumiere */
        queue_str_to_send(USART3,g_M60_L9, TEMPO_SEND);
	if (g_e_State_Light == E_STATE_OFF) {
	    queue_str_to_send(USART3,"OFF\r\n", TEMPO_SEND);
	}
	else {
	    queue_str_to_send(USART3,"ON\r\n", TEMPO_SEND);
	}
	/* HP */
        queue_str_to_send(USART3,g_M60_L10, TEMPO_SEND);
	if (g_e_State_HP == E_STATE_OFF) {
	    queue_str_to_send(USART3,"OFF\r\n", TEMPO_SEND);
	}
	else {
	    queue_str_to_send(USART3,"ON\r\n", TEMPO_SEND);
	}
	/* SYS */
        queue_str_to_send(USART3,g_M60_L11, TEMPO_SEND);
	if (g_e_State_System == E_STATE_OFF) {
	    queue_str_to_send(USART3,"OFF\r\n", TEMPO_SEND);
	}
	else {
	    queue_str_to_send(USART3,"ON\r\n", TEMPO_SEND);
	}
	/* Toucher une touche pour quitter */
        queue_str_to_send(USART3,g_M60_L12, TEMPO_SEND);

}

void Print_Menu_Change_H_EAU_MAX(void){
        queue_str_to_send(USART3,g_CLEAR_SCREEN,TEMPO_SEND);
        queue_str_to_send(USART3,g_M20_L1, TEMPO_SEND);
        queue_str_to_send(USART3,g_M20_L2, TEMPO_SEND);
        queue_str_to_send(USART3,g_M20_L3, TEMPO_SEND);
}

void Print_Menu_Change_T_EAU_MAX(void){
        queue_str_to_send(USART3,g_CLEAR_SCREEN,TEMPO_SEND);
        queue_str_to_send(USART3,g_M21_L1, TEMPO_SEND);
        queue_str_to_send(USART3,g_M21_L2, TEMPO_SEND);
        queue_str_to_send(USART3,g_M21_L3, TEMPO_SEND);
}

void Print_Menu_Change_V_VENT_MAX(void){
        queue_str_to_send(USART3,g_CLEAR_SCREEN,TEMPO_SEND);
        queue_str_to_send(USART3,g_M22_L1, TEMPO_SEND);
        queue_str_to_send(USART3,g_M22_L2, TEMPO_SEND);
        queue_str_to_send(USART3,g_M22_L3, TEMPO_SEND);
}

void Print_Menu_Change_T_EAU(void){
        queue_str_to_send(USART3,g_CLEAR_SCREEN,TEMPO_SEND);
        queue_str_to_send(USART3,g_M30_L1, TEMPO_SEND);
        queue_str_to_send(USART3,g_M30_L2, TEMPO_SEND);
        queue_str_to_send(USART3,g_M30_L3, TEMPO_SEND);
}

void Print_Menu_Change_V_VENT(void){
        queue_str_to_send(USART3,g_CLEAR_SCREEN,TEMPO_SEND);
        queue_str_to_send(USART3,g_M31_L1, TEMPO_SEND);
        queue_str_to_send(USART3,g_M31_L2, TEMPO_SEND);
        queue_str_to_send(USART3,g_M31_L3, TEMPO_SEND);
}

void serial_readwrite_task( void *pvParameters )
{
    serial_str_msg msg;
    char ch;
    int curr_char;
    int done;

    /* Prepare the response message to be queued. */
     if (g_e_Menu_Current == E_MENU_PRINCIPAL) {
	Print_Menu_Main();
    }
    while(1) {
        curr_char = 0;
        done = 0;
        do {

            /* Receive a byte from the RS232 port (this call will block). */
            ch = receive_byte_rtos();

            /* If the byte is an end-of-line type character, then finish the
             * string and indicate we are done.
             */
            if((ch == '\r') || (ch == '\n')) {
		msg.str[curr_char] = curr_char+'0';                
		msg.str[curr_char+1] = '\n';
		msg.str[curr_char+2] = '\r';
                msg.str[curr_char+3] = '\0';
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

        /* Wait. */
        vTaskDelay(100);

	/* Handle received command */
	Handle_Cmd(msg.str);

	/* Afficher le menu */
        if (g_e_Menu_Current == E_MENU_PRINCIPAL) {
	    Print_Menu_Main();
        }
        else if (g_e_Menu_Current == E_MENU_SHOW_VALUE ) {
	    Print_Menu_Value();
        }
        else if (g_e_Menu_Current == E_MENU_CHANGE_H_EAU_MAX ) {
	    Print_Menu_Change_H_EAU_MAX();
        }
        else if (g_e_Menu_Current == E_MENU_CHANGE_T_EAU_MAX ) {
	    Print_Menu_Change_T_EAU_MAX();
        }
        else if (g_e_Menu_Current == E_MENU_CHANGE_V_VENT_MAX ) {
	    Print_Menu_Change_V_VENT_MAX();
        }
        else if (g_e_Menu_Current == E_MENU_CHANGE_T_EAU ) {
	    Print_Menu_Change_T_EAU();
        }
        else if (g_e_Menu_Current == E_MENU_CHANGE_V_VENT ) {
	    Print_Menu_Change_V_VENT();
        }
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
    serial_str_queue_3 = xQueueCreate( 20, sizeof( serial_str_msg ) );
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
    xTaskCreate(serial_readwrite_task, ( signed portCHAR * ) "Serial Read/Write", 512 /* stack size */, NULL, tskIDLE_PRIORITY + 10, NULL );



    /* Start running the tasks. */
    vTaskStartScheduler();

    return 0;
}

void vApplicationTickHook( void )
{
}
