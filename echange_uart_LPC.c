/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/

#define osObjectsPublic                     // define objects in main module
#include "osObjects.h"                      // RTOS object definitions
#include "Driver_USART.h"               // ::CMSIS Driver:USART
#include "Board_LED.h"                  // ::Board Support:LED


extern ARM_DRIVER_USART Driver_USART1;
extern ARM_DRIVER_USART Driver_USART2;

void Init_UART1(void);
void Init_UART2(void);

void tache1(void const * argument);
osThreadId ID_tache1;

void tache1(void const * argument) // tache bluetooth
{
	char depart_recu[2]; // reception du signal de depart
	
	while(1)
	{
		Driver_USART1.Receive(depart_recu,2);
		while(Driver_USART1.GetRxCount()<2); // attente que 1 case soit pleine

		while(Driver_USART2.GetStatus().tx_busy == 1); // attente buffer TX vide
		Driver_USART2.Send(depart_recu,2); // on envoie depuis Rx le contenu du tableau send
		
	}
}

osThreadDef(tache1, osPriorityNormal,1,0);

/*
 * main: initialize and start the system
 */
int main (void) {
  osKernelInitialize ();                    // initialize CMSIS-RTOS

  // initialize peripherals here
	Init_UART1(); // initialisation de l'uart 1
	Init_UART2(); // initialisation de l'uart 2
	LED_Initialize();
  // create 'thread' functions that start executing,
  // example: tid_name = osThreadCreate (osThread(name), NULL);
	ID_tache1 = osThreadCreate ( osThread ( tache1 ), NULL ) ;

			LED_On (1);
		LED_On (2);
		LED_On (3);
  osKernelStart (); // start thread execution 
	osDelay(osWaitForever) ;
}


void Init_UART1(void)
{
	Driver_USART1.Initialize(NULL);
	Driver_USART1.PowerControl(ARM_POWER_FULL);
	Driver_USART1.Control(ARM_USART_MODE_ASYNCHRONOUS |
							ARM_USART_DATA_BITS_8		|
							ARM_USART_STOP_BITS_1		|
							ARM_USART_PARITY_NONE		|
							ARM_USART_FLOW_CONTROL_NONE,
							115200);
	Driver_USART1.Control(ARM_USART_CONTROL_TX,1); // validation emission
	Driver_USART1.Control(ARM_USART_CONTROL_RX,1); // validation transmission
}

void Init_UART2(void)
{
	Driver_USART2.Initialize(NULL);
	Driver_USART2.PowerControl(ARM_POWER_FULL);
	Driver_USART2.Control(ARM_USART_MODE_ASYNCHRONOUS |
							ARM_USART_DATA_BITS_8		|
							ARM_USART_STOP_BITS_1		|
							ARM_USART_PARITY_NONE		|
							ARM_USART_FLOW_CONTROL_NONE,
								115200);
	Driver_USART2.Control(ARM_USART_CONTROL_TX,1); // validation emission
	Driver_USART2.Control(ARM_USART_CONTROL_RX,1); // validation transmission
}