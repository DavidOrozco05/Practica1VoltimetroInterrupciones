#include <math.h>
#include <stdint.h>

#include "mcc_generated_files/mcc.h"

#include "nuestrostimers.h"
#include "nuestroserial.h"
#include "nuestrodisplays.h"

/*-----------------Declaración de DEFINE----------------------*/
#define ADC_PER 200
/*--------------Declaración de Variables----------------------*/
volatile char VGAIT=0;
volatile char VGAI_adc=0;
volatile uint16_t ValADC;
uint16_t datoadc;
unsigned int num_Display;
unsigned int caracter;

void miatenciontmr2(void){
    VGAIT=1;
}

void miatencionadc(void){
    VGAI_adc=1;
}

void main(void)
{
    INTERRUPT_GlobalInterruptEnable();
    INTERRUPT_PeripheralInterruptEnable();
    
	// initialize the device
	SYSTEM_Initialize();
	// inicializar el timer por software de 1 segundo
	Tm_Periodico timer_ADC;
	Tm_Inicie_periodico(&timer_ADC,ADC_PER);
	
	//  inicializar el modulo software de envio de datos
	Sr_Serial miserial;
	inicializar_nuestroserial(&miserial);
	
	//Inicializar el Display Para visualizacion de Datos
	Ds_Display miDisplay;
	inicializar_nuestro_Display(&miDisplay);
    
    TMR2_SetInterruptHandler(miatenciontmr2);
    ADC_SetInterruptHandler(miatencionadc);
    
	
	//Bandera para cambiar entre Pollying e Interrupcion
	//int cambio_Rutina = 0

	char convirtiendo = 0;

	while (1){
		if( VGAIT==1){//Pasaron 10ms
            VGAIT=0;
			Tm_Procese_tiempo (&timer_ADC);//Tm_Procese_tiempo para adc
			mostrar_Display(&miDisplay, num_Display);/*Llamar funcion que maneje display*/
		}
		
		if(Tm_Hubo_periodico(&timer_ADC)){ //Entro Cada segundo
			Tm_Baje_periodico(&timer_ADC);
			ADC_StartConversion();
			convirtiendo = 1;
		}
		if(ADC_IsConversionDone()&&VGAI_adc==1){
            VGAI_adc=0;
			convirtiendo = 0;
			datoadc= ADC_GetConversionResult();
			ADC2Display(&miDisplay, datoadc);//, datoadc);
			//EUSART_Write((char)(datoadc &0x00ff));
			llenar_registros_del_Serial(&miserial,miDisplay.Digito1,miDisplay.Digito2,miDisplay.Digito3);//, Unidad, Decima, Centecima para transmitir serial
		}
		if(EUSART_is_tx_ready() && tengodatos_PorEnviar(&miserial)){//Funcion tengodatos_Enviar en revision
			enviarsiguiente_Dato(&miserial);/*Pose2 ible funcion de Segmentos*/
		}
	}
}