#include <Wire.h>
#include "Arduino.h"
#include "grideye.h"
#include "GE_SoftUart.h"
#include "GE_SoftUartParse.h"

/*******************************************************************************
  variable value definition
*******************************************************************************/
static uint16_t Main_Delay = 85;
       grideye  GE_GridEyeSensor;
       uint8_t  aucThsBuf[2];              /* thermistor temperature        */
       short    g_ashRawTemp[64];          /* temperature of 64 pixels      */
  
/******************************************************************************
  Function：GE_SentDatatoPC
  Description：Used to Sent data to PC
  Input：None
  Output：None
  Others：None
******************************************************************************/
void GE_SentDataToPC( void )
{
    GE_SoftUartPutChar( '*');
    GE_SoftUartPutChar( '*');
    GE_SoftUartPutChar( '*');
    GE_SoftUartPutChar( aucThsBuf[0] );
    GE_SoftUartPutChar( aucThsBuf[1] );
    for(int i = 0; i < 128; i++)
    {
        GE_SoftUartPutChar((char)(*((uint8_t *)(g_ashRawTemp)+i)));
    }
    GE_SoftUartPutChar( '\r');
    GE_SoftUartPutChar( '\n');
}

/******************************************************************************
  Function：GE_SentDatatoPC
  Description：Used to Sent data to PC
  Input：None
  Output：None
  Others：None
******************************************************************************/
void GE_SentDataToPhone( void )
{
      Serial.print("***");
      Serial.write(aucThsBuf[0]);
      Serial.write(aucThsBuf[1]);
      for( int i = 0; i < 128; i++ )
      {
          Serial.write(*((uint8_t *)(g_ashRawTemp)+i));
      }
      Serial.print("\r\n");
}

/******************************************************************************
  Function：GE_SentDatatoPC
  Description：Used to Sent data to PC
  Input ：None
  Output：None
  Others：None 
******************************************************************************/
void GE_SourceDataInitialize( void )
{
  for ( int i = 0; i < 64; i++ )
  {
    g_ashRawTemp[i] = 0xAAAA;
  }
}
/******************************************************************************
  Function：GE_UpdateFerquency
  Description：Used to set the Grid-EYE update frequency
  Input：None
  Output：Grid-EYE frequency
  Return：None
  Others：None
******************************************************************************/
void GE_UpdateFerquency( uint8_t GE_SetFrequency )
{
    switch (GE_SetFrequency)
    {
        case 10:  /* set update frequency 10Hz */
        {
            Main_Delay = 85;
        }
        break;
      
        case 1:   /* set update frequency 1Hz */
        {
        
            Main_Delay = 985;
        }
        break;
      
        default:
        break;
      }
}



void setup()
{
    /* Waiting for BLE Start to finish */
    PIOA->PIO_MDER = 0x00000200;
    delay(1000);

    /* start serial port at 57600 bps:*/
    Serial.begin(57600);
  
    /* Initialize Grid-Eye data interface */
    GE_GridEyeSensor.init( 0 );  // GE_GridEyeSensor is a grideye object, grideye::init() defined in grideye.cpp/.h

    /* Initialize variables  */ 
    GE_SourceDataInitialize( );  // this file - this is why everything is -355 if no data received

    /* Initialize software Software serial port UART1*/
    GE_SoftUartInit( );  // GE_SoftUart.cpp/.h  - send/recieve data between app and grid eye.
}

void loop()
{
    /* Parse of the latest PC sent command */
    GE_CmdParse();  // GE_SoftUartParse.cpp/.h  - used to communicate / send data to grid eye. atm only used to update frequencies from app
    
    /* Get thermistor register value. */
    GE_GridEyeSensor.bAMG_PUB_I2C_Read(0x0E, 2, aucThsBuf );  // GE_GridEyeSensor is a grideye object, grideye::bAMG_PUB_I2C_Read() defined in grideye.cpp/.h

    /* Get temperature register value. */
    for(int i=0;i<4;i++)
    {
        GE_GridEyeSensor.bAMG_PUB_I2C_Read(0x80+32*i, 32, (uint8_t *)g_ashRawTemp+i*32);
        // Assumptions: 4 12C ports/connections/pins/whatever it's called, each pin contains data on 32? pixels
        // i=0: pixels 0-32; i=1: pixels 32-64; i=2 and i=3 not used? (test changing i<4 to i<2 to prove this)
        // g_ashRawTemp is an array of 64 short ints that store raw temp values
        // Important registers: 0x0E-0x0F? (thermistor temp), 0x80 (pixel 1), 0x81 (pixel 2) until 0xBF (pixel 64)
        // OR: other possibility: each temperature value (pixel / thermistor temp) is stored in two registers/uint8s (so pixel 1 = 0x80-0x81, pixel 2 = 0x82-0x83, etc). 2 uint_8 = 1 short int
        // TODO: try printing these raw values to serial and read
    }

     /* Send Grid-Eye sensor data to PC */   
     GE_SentDataToPC( );  // this file

     /* Send Grid-Eye sensor data to phone */ 
     GE_SentDataToPhone( );  // this file (should print to serial atm but can't see it)

     /* set update frequency */
     GE_UpdateFerquency(GE_UpdateFreGet());  //GE_UpdateFerquency() in this file, GE_UpdateFreGet() in GE_SoftUartParse.cpp/.h
     
     delay( Main_Delay );   
}
