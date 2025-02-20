        
//********************************************************************************************
//                                                                            	             *
//      EtherCAT driver for LAN9252                                          	             *
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

 
#include <p18f26k22.h>
#include <delays.h> 
             
#include "main.h"  
#include "easycat.h"     


//---- local functions -------------------------------------------------------------------

void SPIWriteRegisterDirect(unsigned short Address, unsigned long DataOut);
unsigned long SPIReadRegisterDirect(unsigned short Address, unsigned char Len);
  
void SPIWriteRegisterIndirect(unsigned long  DataOut, unsigned short Address, unsigned char Len);
unsigned long SPIReadRegisterIndirect(unsigned short Address, unsigned char Len); 
  
void SPIReadProcRamFifo(void);    
void SPIWriteProcRamFifo(void); 

unsigned char SPI1Transfer (unsigned char Value); 
  

//-----------------------------------------------------------------------------------------  



PROCBUFFER_OUT BufferOut;
PROCBUFFER_IN BufferIn;	



 
//---- EasyCAT board initialization -------------------------------------------------------

unsigned char EasyCAT_Init(void)
{
  #define Tout 1000
   
  ULONG TempLong;
  unsigned short i;

  Delay10KTCYx(200);                  				      // wait 125mS    

  SPIWriteRegisterDirect (RESET_CTL, DIGITAL_RST);        // LAN9252 reset 
   
  i = 0;                                                  // reset timeout 
  do                                                      // wait for reset to complete
  {                                                       //
    i++;                                                  //
    TempLong.Long = SPIReadRegisterDirect (RESET_CTL, 4); //
  }while (((TempLong.Byte[0] & 0x01) != 0x00) && (i != Tout));    
                                                          //                                                       
  if (i == Tout)                                          // time out expired      
  {                                                       // 
    return false;                                         // initialization failed  
  }                                                         
  
  i = 0;                                                  // reset timeout  
  do                                                      // check the Byte Order Test Register
  {                                                       //
    i++;                                                  //      
    TempLong.Long = SPIReadRegisterDirect (BYTE_TEST, 4); //
  }while ((TempLong.Long != 0x87654321) && (i != Tout));  //    
                                                          //                                                            
  if (i == Tout)                                          // time out expired      
  {                                                       // 
    return false;                                         // initialization failed  
  }            
  
  i = 0;                                                  // reset timeout  
  do                                                      // check the Ready flag
  {                                                       //
    i++;                                                  //    
    TempLong.Long = SPIReadRegisterDirect (HW_CFG, 4);    //
  }while (((TempLong.Byte[3] & READY) == 0) && (i != Tout));
                                                          //
  if (i == Tout)                                          // time out expired      
  {                                                       //
    return false;                                         // initialization failed  
  }            												

  for (i=0; i < TOT_BYTE_NUM_IN ; i++)                    // clear the input buffer
  {                                                       //
    BufferIn.Byte[i] = 0;                                 //
  }                                                       //

  return true;                                       	  // initalization completed   
}  


//---- EtherCAT task ------------------------------------------------------------------------------

unsigned char EasyCAT_MainTask(void)                        // must be called cyclically by the application
															
{
  unsigned char WatchDog = 0;
  unsigned Operational = 0; 
  unsigned char i;
  ULONG TempLong; 
  unsigned char Status;  

 
  TempLong.Long = SPIReadRegisterIndirect (WDOG_STATUS, 1); // read the watchdog status
  if ((TempLong.Byte[0] & 0x01) == 0x01)                    //
    WatchDog = 0;                                           // set/reset the corrisponding flag
  else                                                      //
    WatchDog = 1;                                           //
    
  TempLong.Long = SPIReadRegisterIndirect (AL_STATUS, 1);   // read the EtherCAT State Machine status
  Status = TempLong.Byte[0] & 0x0F;                         // to see if we are in operational state
  if (Status == ESM_OP)                                     // 
    Operational = 1;                                        //
  else                                                      // set/reset the corrisponding flag
    Operational = 0;                                        //    


                                                            //--- process data transfert ----------
                                                            //                                                        
  if (WatchDog == 1 | Operational == 0)                     // if watchdog is active or we are 
  {                                                         // not in operational state, reset 
    for (i=0; i < TOT_BYTE_NUM_OUT ; i++)                   // the output buffer
    {                                                       //
      BufferOut.Byte[i] = 0;                                //
    }                                                       //
  }															//
  														
  else                                                     	// otherwise transfer process data from 	
  {                                                         // the EtherCAT core to the output buffer 
    SPIReadProcRamFifo();                                   // 
  }                                                         // 
                 						
  SPIWriteProcRamFifo();                                    // we always transfer process data from
                                                            // the input buffer to the EtherCAT core  

  if (WatchDog)                                             // return the status of the State Machine      
  {                                                         // and of the watchdog
    Status |= 0x80;                                         //
  }                                                         //
  return Status;                                            //     
}

    
//---- read a directly addressable registers  -----------------------------------------------------

unsigned long SPIReadRegisterDirect (unsigned short Address, unsigned char Len)

                                                   // Address = register to read
                                                   // Len = number of bytes to read (1,2,3,4)
                                                   //
                                                   // a long is returned but only the requested bytes
                                                   // are meaningful, starting from LsByte                                                 
{
  ULONG Result; 
  UWORD Addr;
  unsigned char i;

  Addr.Word = Address; 
 

  SCS_Low_macro                                             // SPI chip select enable

  SPI_TransferTx(COMM_SPI_READ);                            // SPI read command
  SPI_TransferTx(Addr.Byte[1]);                             // address of the register
  SPI_TransferTxLast(Addr.Byte[0]);                         // to read, MsByte first
 
  for (i=0; i<Len; i++)                                     // read the requested number of bytes
  {                                                         // LsByte first 
    Result.Byte[i] = SPI_TransferRx(DUMMY_BYTE);            //
  }                                                         //    
  
  SCS_High_macro                                            // SPI chip select disable 
 
  return Result.Long;                                       // return the result
}


//---- write a directly addressable registers  ----------------------------------------------------

void SPIWriteRegisterDirect (unsigned short Address, unsigned long DataOut)

                                                   // Address = register to write
                                                   // DataOut = data to write
{ 
  ULONG Data; 
  UWORD Addr;
  Addr.Word = Address;
  Data.Long = DataOut;    

  
  SCS_Low_macro                                             // SPI chip select enable  
  
  SPI_TransferTx(COMM_SPI_WRITE);                           // SPI write command
  SPI_TransferTx(Addr.Byte[1]);                             // address of the register
  SPI_TransferTx(Addr.Byte[0]);                             // to write MsByte first

  SPI_TransferTx(Data.Byte[0]);                             // data to write 
  SPI_TransferTx(Data.Byte[1]);                             // LsByte first
  SPI_TransferTx(Data.Byte[2]);                             //
  SPI_TransferTxLast(Data.Byte[3]);                         //
 
  SCS_High_macro                                            // SPI chip select enable   
}


//---- read an indirectly addressable registers  --------------------------------------------------

unsigned long SPIReadRegisterIndirect (unsigned short Address, unsigned char Len)

                                                   // Address = register to read
                                                   // Len = number of bytes to read (1,2,3,4)
                                                   //
                                                   // a long is returned but only the requested bytes
                                                   // are meaningful, starting from LsByte                                                  
{
  ULONG TempLong;
  UWORD Addr;
  Addr.Word = Address;
                                                            // compose the command
                                                            //
  TempLong.Byte[0] = Addr.Byte[0];                          // address of the register
  TempLong.Byte[1] = Addr.Byte[1];                          // to read, LsByte first
  TempLong.Byte[2] = Len;                                   // number of bytes to read
  TempLong.Byte[3] = ESC_READ;                              // ESC read 

  SPIWriteRegisterDirect (ECAT_CSR_CMD, TempLong.Long);     // write the command

  do
  {                                                         // wait for command execution
    TempLong.Long = SPIReadRegisterDirect(ECAT_CSR_CMD,4);  //
  }                                                         //
  while(TempLong.Byte[3] & ECAT_CSR_BUSY);                  //
                                                             
                                                              
  TempLong.Long = SPIReadRegisterDirect(ECAT_CSR_DATA,Len); // read the requested register
  return TempLong.Long;                                     //
}


//---- write an indirectly addressable registers  -------------------------------------------------

void  SPIWriteRegisterIndirect (unsigned long DataOut, unsigned short Address, unsigned char Len)

                                                   // Address = register to write
                                                   // DataOut = data to write                                                    
{
  ULONG TempLong;
  UWORD Addr;
  Addr.Word = Address;

  
  SPIWriteRegisterDirect (ECAT_CSR_DATA, DataOut);            // write the data

                                                              // compose the command
                                                              //                                
  TempLong.Byte[0] = Addr.Byte[0];                            // address of the register  
  TempLong.Byte[1] = Addr.Byte[1];                            // to write, LsByte first
  TempLong.Byte[2] = Len;                                     // number of bytes to write
  TempLong.Byte[3] = ESC_WRITE;                               // ESC write

  SPIWriteRegisterDirect (ECAT_CSR_CMD, TempLong.Long);       // write the command

  do                                                          // wait for command execution
  {                                                           //
    TempLong.Long = SPIReadRegisterDirect (ECAT_CSR_CMD, 4);  //  
  }                                                           //  
  while (TempLong.Byte[3] & ECAT_CSR_BUSY);                   //
}


//---- read from process ram fifo ----------------------------------------------------------------

void SPIReadProcRamFifo(void)         // read data from the output process ram, through the fifo
                                      //        
                                      // these are the bytes received from the EtherCAT master and
                                      // that will be use by our application to write the outputs
{
  ULONG TempLong;
  unsigned char i;
  
  
  #if TOT_BYTE_NUM_OUT > 0

    SPIWriteRegisterDirect (ECAT_PRAM_RD_CMD, PRAM_ABORT);        // abort any possible pending transfer

  
	SPIWriteRegisterDirect (ECAT_PRAM_RD_ADDR_LEN, (0x00001000 | (((uint32_t)TOT_BYTE_NUM_OUT) << 16)));   
                                                                  // the high word is the num of bytes
                                                                  // to read 0xTOT_BYTE_NUM_OUT----
                                                                  // the low word is the output process        
                                                                  // ram offset 0x----1000 

    SPIWriteRegisterDirect (ECAT_PRAM_RD_CMD, 0x80000000);        // start command        
 
 
                                                //------- one round is enough if we have ----
                                                //------- to transfer up to 64 bytes --------
   
    do                                                            // wait for the data to be       
    {                                                             // transferred from the output  
      TempLong.Long = SPIReadRegisterDirect (ECAT_PRAM_RD_CMD,2); // process ram to the read fifo       
    }                                                             //    
    while (TempLong.Byte[1] != (FST_BYTE_NUM_ROUND_OUT/4));       // *CCC* 
  
    SCS_Low_macro                                                 // enable SPI chip select 
  
    SPI_TransferTx(COMM_SPI_READ);                                // SPI read command
    SPI_TransferTx(0x00);                                         // address of the read  
    SPI_TransferTxLast(0x00);                                     // fifo MsByte first
  
    for (i=0; i< FST_BYTE_NUM_ROUND_OUT; i++)                     // transfer the data
    {                                                             //
      BufferOut.Byte[i] = SPI_TransferRx(DUMMY_BYTE);             //
    }                                                             //
    
    SCS_High_macro                                                // disable SPI chip select    
  #endif  

  
  #if SEC_BYTE_NUM_OUT > 0                    //-- if we have to transfer more then 64 bytes --
                                              //-- we must do another round -------------------
                                              //-- to transfer the remainig bytes -------------


    do                                                          // wait for the data to be       
    {                                                           // transferred from the output  
      TempLong.Long = SPIReadRegisterDirect(ECAT_PRAM_RD_CMD,2);// process ram to the read fifo 
    }                                                           //    
    while (TempLong.Byte[1] != SEC_BYTE_NUM_ROUND_OUT/4);       // *CCC*  

    SCS_Low_macro                                               // enable SPI chip select   
    
    SPI_TransferTx(COMM_SPI_READ);                              // SPI read command
    SPI_TransferTx(0x00);                                       // address of the read  
    SPI_TransferTxLast(0x00);                                   // fifo MsByte first
    
    for (i=0; i< (SEC_BYTE_NUM_ROUND_OUT); i++)                 // transfer loop for the remaining 
    {                                                           // bytes
      BufferOut.Byte[i+64] = SPI_TransferRx(DUMMY_BYTE);        // we transfer the second part of
    }                                                           // the buffer, so offset by 64
      
    SCS_High_macro                                              // SPI chip select disable  
  #endif  
}  


//---- write to the process ram fifo --------------------------------------------------------------

void SPIWriteProcRamFifo()             // write data to the input process ram, through the fifo
                                       //    
                                       // these are the bytes that we have read from the inputs of our                   
                                       // application and that will be sent to the EtherCAT master
{
  ULONG TempLong;
  unsigned char i;
  
  
  #if TOT_BYTE_NUM_IN > 0  
  
    SPIWriteRegisterDirect (ECAT_PRAM_WR_CMD, PRAM_ABORT);        // abort any possible pending transfer
  
 
    SPIWriteRegisterDirect (ECAT_PRAM_WR_ADDR_LEN, (0x00001200 | (((uint32_t)TOT_BYTE_NUM_IN) << 16)));   
                                                                  // the high word is the num of bytes
                                                                  // to write 0xTOT_BYTE_NUM_IN----
                                                                  // the low word is the input process        
                                                                  // ram offset  0x----1200
                                                                                               
    SPIWriteRegisterDirect (ECAT_PRAM_WR_CMD, 0x80000000);        // start command  
  
  
                                                //------- one round is enough if we have ----
                                                //------- to transfer up to 64 bytes --------
    
    do                                                            // check that the fifo has      
    {                                                             // enough free space 
      TempLong.Long = SPIReadRegisterDirect (ECAT_PRAM_WR_CMD,2); //  
    }                                                             //  
    while (TempLong.Byte[1] <   (FST_BYTE_NUM_ROUND_IN/4));       //    *CCC*
  
    SCS_Low_macro                                                 // enable SPI chip select
  
    SPI_TransferTx(COMM_SPI_WRITE);                               // SPI write command
    SPI_TransferTx(0x00);                                         // address of the write fifo 
    SPI_TransferTx(0x20);                                         // MsByte first 

    for (i=0; i< (FST_BYTE_NUM_ROUND_IN - 1 ); i++)               // transfer the data
    {                                                             //
      SPI_TransferTx (BufferIn.Byte[i]);                          //      
    }                                                             //
                                                                  //  
    SPI_TransferTxLast (BufferIn.Byte[i]);                        // one last byte
  
    SCS_High_macro                                                // disable SPI chip select           
  #endif        

  
  #if SEC_BYTE_NUM_IN > 0                     //-- if we have to transfer more then 64 bytes --
                                              //-- we must do another round -------------------
                                              //-- to transfer the remainig bytes -------------

    do                                                          // check that the fifo has     
    {                                                           // enough free space       
      TempLong.Long = SPIReadRegisterDirect(ECAT_PRAM_WR_CMD,2);// 
    }                                                           //  
    while (TempLong.Byte[1] < (SEC_BYTE_NUM_ROUND_IN/4));       //   *CCC*
                             
    SCS_Low_macro                                               // enable SPI chip select
    
    SPI_TransferTx(COMM_SPI_WRITE);                             // SPI write command
    SPI_TransferTx(0x00);                                       // address of the write fifo 
    SPI_TransferTx(0x20);                                       // MsByte first 

    for (i=0; i< (SEC_BYTE_NUM_ROUND_IN - 1); i++)              // transfer loop for the remaining 
    {                                                           // bytes
      SPI_TransferTx (BufferIn.Byte[i+64]);                     // we transfer the second part of
    }                                                           // the buffer, so offset by 64
                                                                //  
    SPI_TransferTxLast (BufferIn.Byte[i+64]);                   // one last byte  

    SCS_High_macro                                              // disable SPI chip select    
  #endif     
}



//- SPI 1 data transfer -------------------------------------------------------------------

unsigned char SPI1Transfer (unsigned char Value)

{
    unsigned char Result;

    SSP1BUF = (Value);
    while(!SSP1STATbits.BF){};
    Result = SSP1BUF;

    return Result;
}
