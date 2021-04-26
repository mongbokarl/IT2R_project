/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/

#define osObjectsPublic                     // define objects in main module
#include "osObjects.h"                      // RTOS object definitions
#include "Driver_USART.h"               // ::CMSIS Driver:USART
#include "LPC17xx.h"                    // Device header
#include "Driver_SPI.h"                 // ::CMSIS Driver:SPI
#include "Board_LED.h"                  // ::Board Support:LED
#include "stdio.h"


extern ARM_DRIVER_USART Driver_USART1;

char tab[2], qui, parametre; 	// tab[2] pour recevoir la consigne de l'uart: consigne => qui (vit ou dir) & parametre (quel vitesse ou direction)
char vitesse, direction;
int n=400; 										// variable n pour faire varier le raport cyclique de le PWM du LIDAR

void Init_UART1(void);				// Initialisation de l'UART0 sur CALLBACK
void USART1_callback (uint32_t event);	// Fonction a exécuté si CALLBACK: active le thread USART0
void USART1(void const * argument);			// fornit le tableau tab[2], les variables Vitesse et Direction, active la fonction PWM_Vit_Dir

void Init_IRQ_PWM_LIDAR(void);	//créée des inteructions et active TIMER0_IRQHandler
void TIMER0_IRQHandler(void);		// inverse l'état de la broche P0.0 pour fabriquer une PWM

void Init_PWM_Vit_Dir (void);		//Initialise les ports P2.3(vitesse) et P2.4(direction) pour fornir des PWM de 50Hz
void PWM_Vit_Dir ( void const* argument); //modifie le rapport Cyclique de la Vitesse et de la Direction en fontion des valeurs reçus avec l'USART

osThreadId ID_PWM_Vit_Dir, ID_USART1;

osThreadDef (USART1, osPriorityNormal, 1, 0);
osThreadDef (PWM_Vit_Dir, osPriorityNormal, 1, 0);

/*
 * main: initialize and start the system
 */
int main (void) {
  osKernelInitialize ();                    // initialize CMSIS-RTOS
	
	  // initialize peripherals here
	LED_Initialize();
	Init_UART1();
	Init_IRQ_PWM_LIDAR();
	Init_PWM_Vit_Dir();

  // create 'thread' functions that start executing,
  // example: tid_name = osThreadCreate (osThread(name), NULL);
	ID_USART1 = osThreadCreate ( osThread ( USART1 ), NULL ) ;
	ID_PWM_Vit_Dir = osThreadCreate ( osThread (PWM_Vit_Dir), NULL) ;
	
  osKernelStart ();                         // start thread execution 
	osWaitForever;
}


void Init_UART1(void)		// Initialisation de l'UART0 sur CALLBACK
	{
			Driver_USART1.Initialize(USART1_callback);
			Driver_USART1.PowerControl(ARM_POWER_FULL);
			Driver_USART1.Control(ARM_USART_MODE_ASYNCHRONOUS |
															ARM_USART_DATA_BITS_8        |
															ARM_USART_STOP_BITS_1        |
															ARM_USART_PARITY_NONE        |
															ARM_USART_FLOW_CONTROL_NONE,
															230400);
			Driver_USART1.Control(ARM_USART_CONTROL_TX,1); // validation emission
			Driver_USART1.Control(ARM_USART_CONTROL_RX,1); // validation transmission
	}
	
void USART1_callback(uint32_t event)
{
	osSignalSet(ID_USART1, 0x01);
	osDelay(10);
}
void USART1(void const * argument)		// Fonction a exécuté si CALLBACK: active le thread USART0
	{
		while(1)
	 {
		osSignalWait(0x01, osWaitForever);
		Driver_USART1.Receive(tab,2);
		while(Driver_USART1.GetRxCount()<2); // attente que 1 case soit pleine
		LED_On(0);
		 if(vitesse==9) vitesse=5;
		 else if(vitesse==5) vitesse=0;
		 else vitesse=9;
		 
		 if(direction==1) direction=2;
		 else if(direction==2) direction=3;
		 else if(direction==3) direction=4;
		 else if(direction==4) direction=5;
		 else direction=1;
			LED_On(1);
		 osDelay(10);
		 
		/*	osSignalSet(ID_PWM_Vit_Dir, 0x02);
		 vitesse=5;
		 LED_On(2);
		 osDelay(10);
		 
		 osSignalSet(ID_PWM_Vit_Dir, 0x03);
		 vitesse=0;
		 LED_On(3);
		 osDelay(10);*/
	 }
	}

	
void Init_IRQ_PWM_LIDAR(void)			//créée des inteructions et active TIMER0_IRQHandler
	{
		//LED_On(0);
		LPC_GPIO0->FIODIR0 = 0x01;
		LPC_TIM0->CTCR = 0; // Mode Timer (signaux carrés)
		LPC_TIM0->PR = 0; // Prescaler à 0
		LPC_TIM0->MR0 = n-1; // valeur de N (=souvent)
		// RAZ si correspondance avec MR0, et interruption
		
		LPC_TIM0->MCR = LPC_TIM0->MCR | (3<<0);
		NVIC_SetPriority(TIMER0_IRQn,0);
		 // TIMER1 : IT de priorité 0
		
		NVIC_EnableIRQ(TIMER0_IRQn); // active IT du TIMER1
		LPC_TIM0->TCR = 1; // démarre le comptage du Timer
	}
	
void TIMER0_IRQHandler(void)			// inverse l'état de la broche P0.0 pour fabriquer une PWM
	{
		//LED_On(1);
		// Acquittement pour le Match Register 0 du timer 0
		LPC_TIM0->IR = LPC_TIM0->IR | (1<<0);
		if (n==400) n=600;
		else if(n==600) n=400;
		LPC_TIM0->MR0 = n-1;
		LPC_TIM0->TCR = 1;
		
		LPC_GPIO0->FIOPIN0 ^= 0x01;
	}
	
void Init_PWM_Vit_Dir (void)			//Initialise les ports P2.3(vitesse) et P2.4(direction) pour fornir des PWM de 50Hz
{
	//LED_On(0);
	/*PWM 50Hz pour le servo et le vatiateur*/
		LPC_SC->PCONP = LPC_SC->PCONP | (1<<6);
		LPC_PINCON->PINSEL4 |= (101<<6);
		LPC_PWM1->CTCR =0;	//mode timer
		LPC_PWM1->PR =0; //Pas de prédivision
		LPC_PWM1->MR0 =499999; //valeur de N 50Hz
		LPC_PWM1->MCR =LPC_PWM1->MCR | (1<<1); //RAZ du compteur si correspondance avec MR0	
		
		LPC_PWM1->PCR =LPC_PWM1->PCR | (1<<12); //active PWM3
		LPC_PWM1->PCR =LPC_PWM1->PCR | (1<<13); //active PWM3
		
		LPC_PWM1->MR4 = 35999;// rapport cyclique vitesse sur P2.3
		LPC_PWM1->MR5 = 45000;// rapport cyclique direction sur P2.4
		
		LPC_PWM1->TCR =1;	//démarage du timer
}

void PWM_Vit_Dir (void const * argument)		//modifie le rapport Cyclique de la Vitesse et de la Direction en fontion des valeurs reçus avec l'USART
{
	LED_On(3);
	while(1)
		{
			if(vitesse <=9 && vitesse >=0) LPC_PWM1->MR4 = 40000-(vitesse*1000);// rapport cyclique vitesse sur P2.3
			
			//LPC_PWM1->MR5 = N;// rapport cyclique direction sur P2.4
			if(direction==1) LPC_PWM1->MR5 = 56000;
			else if(direction==2) LPC_PWM1->MR5 = 53250;
			else if(direction==3) LPC_PWM1->MR5 = 50000;
			else if(direction==4) LPC_PWM1->MR5 = 46750;
			else if(direction==5) LPC_PWM1->MR5 = 43500;
				
			LPC_PWM1->TCR =1;	//démarage du timer
			//osSignalWait(0x01, osWaitForever);
		}
}
