//********************************************************************************************    
//                                                                                           *
// This software is distributed as an example, "AS IS", in the hope that it could            *
// be useful, WITHOUT ANY WARRANTY of any kind, express or implied, included, but            *
// not limited,  to the warranties of merchantability, fitness for a particular              *
// purpose, and non infringiment. In no event shall the authors be liable for any            *    
// claim, damages or other liability, arising from, or in connection with this software.     *
//                                                                                           *
//******************************************************************************************** 



#ifndef MAIN_H
#define MAIN_H


//------ data types definitions ------------------------------------------------------------------

#define uint8_t 	unsigned char
#define uint16_t 	unsigned short
#define uint32_t 	unsigned long
#define int8_t 		signed char
#define int16_t 	signed short
#define int32_t 	signed long

//------------------------------------------------------------------------------------------------

#define true 1
#define false 0

//-----------------------------------------------------------------------------------------------


/*

#define CUSTOM


//*********************************************************************************************

#define CUSTOM                      // Custom mode
#include "TestEasyCAT_Custom.h"     // This file has been created by the Easy Configurator 
                                    // and must be located in the Arduino project folder
                                    //
                                    // There are two others files created by the Easy Configurator:
                                    // TestEasyCAT_Custom.bin that must be loaded into the EEPROM.
  */                                  // TestEasyCAT_Custom.xml that must be used by the EtherCAT master. 
                                    
//*********************************************************************************************




#define	Led	            LATBbits.LATB7                      // run led
                                                           
#define	Spi1Cs_	        LATCbits.LATC2						// SPI 1 chip select

                                                            // --- digital output pins (leds) --------------------------
#define Out_0           LATBbits.LATB5
#define Out_1           LATBbits.LATB4
#define Out_2           LATBbits.LATB3
#define Out_3           LATBbits.LATB2
#define Out_4           LATBbits.LATB1
#define Out_5           LATBbits.LATB0
#define Out_6           LATCbits.LATC7
#define Out_7           LATCbits.LATC6

                                                            // --- digital input pins (dip switches) -------------------
#define In_0            PORTAbits.RA2
#define In_1            PORTAbits.RA3
#define In_2            PORTAbits.RA4
#define In_3            PORTAbits.RA5
#define In_4            PORTAbits.RA7
#define In_5            PORTAbits.RA6
#define In_6            PORTCbits.RC0
#define In_7            PORTCbits.RC1

//---------------------------------------------------------------------------------------


#endif



