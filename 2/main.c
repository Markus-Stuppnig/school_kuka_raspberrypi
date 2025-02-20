//********************************************************************************************
//                                                                            	             *
//	EasyCAT PRO custom example for Microchip PIC18F26K22                         	         *  
//  VERSION 2.0    16-09-18                                                 	             *
//                                                                            	             *  
//********************************************************************************************


//********************************************************************************************    
//                                                                                           *
// This software is distributed as an example, "AS IS", in the hope that it could            *
// be useful, WITHOUT ANY WARRANTY of any kind, express or implied, included, but            *
// not limited,  to the warranties of merchantability, fitness for a particular              *
// purpose, and non infringiment. In no event shall the authors be liable for any            *    
// claim, damages or other liability, arising from, or in connection with this software.     *
//                                                                                           *
//******************************************************************************************** 



// In this example the variables exchanged with the EtherCAT master have been 
// customized using the Easy Configuration tool. 
// To understand how to do this please see the Easy Configurator user manual.
//
//
// The input variables used in this example are:
//
//		uint16_t    Analog_0                The first analog input         
//		uint16_t    Analog_1                The second analog input
//		uint8_t     DipSwitches             The eight dip switches
//		uint8_t     Bit8_FallingTestRamp    A falling test ramp
//		uint16_t    Bit16_RisingTestRamp    A rising test ramp
//
// And the output:
//
//		uint8_t     Leds;                   The eight leds

// The "CUSTOM" define is in the easycat.h file



#include <p18f26k22.h>
#include <delays.h> 

#include "main.h"
#include "easycat.h"


//--- cpu configuration bits -------------------------------------------------------------

#pragma	config	FOSC 	= INTIO67      		// Internal oscillator block, port function on RA6 and RA7
#pragma	config	PLLCFG  = ON                // Oscillator multiplied by 4 

#pragma	config  PRICLKEN = ON               // Primary clock enabled 
#pragma	config  FCMEN = OFF                 // Fail-Safe Clock Monitor disabled 
#pragma	config  IESO = OFF                  // Oscillator Switchover mode disabled 

#pragma	config  PWRTEN = ON                 // Power up timer enabled 
#pragma	config  BOREN = SBORDIS             // Brown-out Reset enabled in hardware only (SBOREN is disabled) 
#pragma	config  BORV = 285                  // BORV = 285 VBOR set to 2.85 V nominal 

#pragma	config  WDTEN = OFF                 // Watch dog timer is always disabled. SWDTEN has no effect. 

#pragma	config  MCLRE = EXTMCLR             // MCLR pin enabled, RE3 input pin disabled

#pragma	config  STVREN = ON                 // Stack full/underflow will cause Reset 
#pragma	config  LVP 	= OFF       		// Single-Supply ICSP disabled

#pragma	config  XINST 	= OFF       		// Instruction set extension and Indexed Addressing mode disabled (Legacy mode)
#pragma	config  DEBUG   = OFF       		// Background debugger disabled, RB6 and RB7 configured as general purpose I/O pins


//--- local functions ------------------------------------------------------------------

void PicInit (void);
void Application (void);
uint16_t ReadTimer0(void);


//--------------------------------------------------------------------------------------

extern PROCBUFFER_OUT BufferOut;			// EtherCAT buffer
extern PROCBUFFER_IN BufferIn;				//


uint8_t EcatState;							// EtherCAT state

uint8_t bit8_TestDownCounter;				// test counters			
uint16_t bit16_TestUpCounter;				//

uint16_t PreviousTime;						// system time
uint16_t Time;								// 


//-------------------------------------------------------------------------------------//
//																				       //
//		Main																      	   //
//																				       //			
//-------------------------------------------------------------------------------------//


//----- initialization -----------------------------------------------------------------

void main (void)  

{
	PicInit();				                    		// PIC initialization 

    if  (EasyCAT_Init() == false)               		// EasyCAT board initialization 
    {                                           		// if the initialization fails we stay 
        while (1)                               		// in loop for ever, blinking the led
        {                                       		//
            Led = 1;                            		//    
            Delay10KTCYx(200);                  		// wait 125mS   
            Led = 0;                            		//
            Delay10KTCYx(200);                  		// wait 125mS
        } 
    }
            
    else                                        		// EasyCAT board correcltly initialized
    {                                           		//    
        Led = 1;                                		// three flash on the led   
        Delay10KTCYx(200);                      		// wait 125mS   
        Led = 0;                                		//
        Delay10KTCYx(200);                      		// wait 125mS

        Led = 1;                                		//    
        Delay10KTCYx(200);                      		// wait 125mS   
        Led = 0;                                		//
        Delay10KTCYx(200);                      		// wait 125mS

        Led = 1;                                		//    
        Delay10KTCYx(200);                      		// wait 125mS   
        Led = 0;                                		//
        Delay10KTCYx(200);                      		// wait 125mS
    }

	Time = ReadTimer0();


//----- main loop ----------------------------------------------------------------------
   
    while(1)
    {
        EcatState = EasyCAT_MainTask();              	// EtherCAT task: here we exchange the data with the EtherCAT master                                                    

		Time = ReadTimer0();							// In this example we call the user application every 20mS 
		if (Time - PreviousTime > (20000/16))			//
		{												//
			PreviousTime = Time;						//
														//
			Application();								// user application
    	}												//			
	}
}



//-------------------------------------------------------------------------------------//
//																				       //
//		Functions     														      	   //
//																				       //			
//-------------------------------------------------------------------------------------//


void Application (void)
{
	uint8_t Leds;
	uint8_t DipSwitches;

    if (EcatState == 0x08)                              // If the EasyCAT is in Operational State
    {                                                   // and the communication is running
   		Led = 1;                      					// light on the run led
    }													//
	else												//	
	{													//
   		Led = 0;										//		
	}													//


	Leds = BufferOut.Cust.Leds;							//---- 8 leds output management
														//
    if (Leds & 0b00000001)           					//		
        Out_0 = 1;                                  	//
    else                                           	 	//
        Out_0 = 0;                                  	//
    if (Leds & 0b00000010)           					//
        Out_1 = 1;                                  	//
    else                                            	//        
        Out_1 = 0;                                  	//
    if (Leds & 0b00000100)           					//
        Out_2 = 1;                                  	//
    else                                            	//        
        Out_2 = 0;                                  	//
    if (Leds & 0b00001000)           					//
        Out_3 = 1;                                  	//
    else                                            	//
        Out_3 = 0;                                  	//
    if (Leds & 0b00010000)           					//
        Out_4 = 1;                                  	//
    else                                            	//
        Out_4 = 0;                                  	//
    if (Leds & 0b00100000)           					//        
        Out_5 = 1;                                  	//
    else                                            	//    
        Out_5 = 0;                                  	//
    if (Leds & 0b01000000)           					//
        Out_6 = 1;                                  	//
    else                                            	//
        Out_6 = 0;                                  	//
    if (Leds & 0b10000000)           					//
        Out_7 = 1;                                  	//
    else                                            	//
        Out_7 = 0;                                  	//


    DipSwitches = 0x00;               					//---- 8 dip switches input management    
														//
    if (!In_0)                                      	//
        DipSwitches |= 0b00000001;    					//
    if (!In_1)                                      	//        
        DipSwitches |= 0b00000010;    					//    
    if (!In_2)                                      	//
        DipSwitches |= 0b00000100;    					//
    if (!In_3)                                      	//
        DipSwitches |= 0b00001000;    					//    
    if (!In_4)                                      	//
        DipSwitches |= 0b00010000;    					//
    if (!In_5)                                      	//
        DipSwitches |= 0b00100000;    					//
    if (!In_6)                                      	//
        DipSwitches |= 0b01000000;    					//
    if (!In_7)                                      	//
        DipSwitches |= 0b10000000;    					//
														//	
	BufferIn.Cust.DipSwitches = DipSwitches;			//


    ADCON0bits.CHS = 0b00000 ;                      	//---- analog input "0" management
    ADCON0bits.DONE = 1;                            	// start conversion
    while (ADCON0bits.DONE){};                      	// wait for conversion completed
    BufferIn.Cust.Analog_0 = ADRES << 6;              	// normalize the result to 16 bits

    ADCON0bits.CHS = 0b00001 ;                      	//---- analog input "1" management
    ADCON0bits.DONE = 1;                            	//
    while (ADCON0bits.DONE){};                      	//
    BufferIn.Cust.Analog_1 = ADRES << 6;          		//   


	bit8_TestDownCounter--;										    //---- test counters management
	bit16_TestUpCounter += 0x003F;									//
																	//
    BufferIn.Cust.Bit8_FallingTestRamp = bit8_TestDownCounter;      //
    BufferIn.Cust.Bit16_RisingTestRamp = bit16_TestUpCounter;		//
}



//------- PIC initialization -----------------------------------------------------------
	
void PicInit (void)

{
												//---- CLOCK configuration -----------------
    OSCCON  = 0x00;                         	//
    OSCCONbits.IRCF = 0b111;                	// HFINTOSC Oscillator 16Mhz
												//
    OSCTUNE = 0x00;								//
	OSCTUNEbits.PLLEN = 1;						// PLLx4 enabled    fosc = 16MHz*4 = 64MHz 

    while (!OSCCONbits.HFIOFS){}            	// wait until HFINTOSC is stable


												//---- I/O port configuration --------------
                                            	//
    ANSELA = 0b00000011;                   	 	// all the i/o are digital                
    ANSELB = 0b00000000;                    	// except RA0 and RA1 that are analog
    ANSELC = 0b00000000;                    	//

    TRISA = 0b11111111;							// set pins direction
												//
    LATB  = 0b00000000;							//
    TRISB = 0b00000000;							//
												//
    LATC  = 0b00000100;							//
    TRISC = 0b00010011; 						//

												//---- ADC configuration -------------------
												//
    ADCON1 = 0x00;                          	// VEF+ VDD   VREF- = VSS
												//
    ADCON2 = 0x00;                          	//
    ADCON2bits.ADFM = 1;                    	// result is right justified
    ADCON2bits.ADCS = 0b101;                	// AD clock fosc/16  = 16MHz/16 = 1MHz   
    ADCON2bits.ACQT = 0b111;                	// acquisition time 20*Tad  = 20uS    
												//
    ADCON0bits.ADON = 1;                    	// ADC on


    											//---- SPI1 configuration-------------------
    											//
                                            	// the SPI1 is used to communicate with the EasyCAT board                  
    SSP1STAT = 0x00;                        	// and so it it set to SPI master
    SSP1STATbits.CKE = 1;                   	// transmission occurs on transition from active to idle clock state
                                            	// (falling edge)        
    SSP1CON1 = 0x00;                        	//    
    SSP1CON1bits.CKP = 0;                   	// idle state for the clock is a low level 
                                            	//
    SSP1CON1bits.SSPM = 0b0001;             	// clock FOSC/16  = 64MHz/16 = 4MHz   
    SSP1CON1bits.SSPEN = 1;                 	// enable SPI


    											//---- Timer 0 configuration -----------------------------
    											//
                                            	// used for system time
                                            	//    
    T0CON = 0x00;                          	 	// internal instruction cycle clock
    T0CONbits.T0PS = 0b111;                 	// prescaler /256 = timer clock = 16uS   
    T0CONbits.T08BIT = 0;                   	// 16 bit   
    T0CONbits.TMR0ON = 1;						// start Timer 0
}


//------ read Timer 0 ----------------------------------------------------------------------------------------------

uint16_t ReadTimer0(void)					 
{
	uint16_t Timer0Value;
	
	Timer0Value = (uint16_t)TMR0L;				// respect the sequence TMR0L TM0H !!!	
	Timer0Value |=  ((uint16_t)TMR0H) << 8; 	// (see PIC18F26K22 datasheet)	
												//
	return Timer0Value;							//
}

