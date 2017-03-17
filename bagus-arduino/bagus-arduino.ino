#include <Wire.h>
#include "Arduino.h"
#include "grideye.h"

/*******************************************************************************
  variable value definition
*******************************************************************************/
grideye  GE_GridEyeSensor;

/*******************************************************************************
  methods
 ******************************************************************************/
void GE_SourceDataInitialize(short *data) {
  for ( int i = 0; i < SNR_SZ; i++ ) {
    data[i] = 0;
  }
}

void GE_SourceDataInitializeF(float *data) {
  for ( int i = 0; i < SNR_SZ; i++ ) {
    data[i] = 0.0;
  }
}

void getTempArray(float *thermistorTemp, float *pixelsTemp) {
  // Get thermistor register value
  UCHAR aucThsBuf[GRIDEYE_REGSZ_THS];
  GE_GridEyeSensor.bAMG_PUB_I2C_Read(GRIDEYE_REG_THS00, GRIDEYE_REGSZ_THS, aucThsBuf);

  // Get temperature register value
  UCHAR aucTmpBuf[128];
  for(int i=0; i<4; i++) {
    GE_GridEyeSensor.bAMG_PUB_I2C_Read(GRIDEYE_REG_TMP00+32*i, 32, aucTmpBuf+32*i);
  }
  
  // Convert raw values to short
  short g_shThsTemp = shAMG_PUB_TMP_ConvThermistor(aucThsBuf);

  short g_ashRawTemp[SNR_SZ];
  GE_SourceDataInitialize(g_ashRawTemp);
  vAMG_PUB_TMP_ConvTemperature64(aucTmpBuf, g_ashRawTemp);
  
  // Convert short values to float
  *thermistorTemp = fAMG_PUB_CMN_ConvStoF(g_shThsTemp);
  
  GE_SourceDataInitializeF(pixelsTemp);
  for ( int i = 0; i < SNR_SZ; i++ ) {
    pixelsTemp[i] = fAMG_PUB_CMN_ConvStoF(g_ashRawTemp[i]);
  }
}

/*******************************************************************************
  arduino
 ******************************************************************************/
void setup() {
  // put your setup code here, to run once:

  // Waiting for BLE Start to finish (not sure if this is needed)
  PIOA->PIO_MDER = 0x00000200;
  delay(1000);
  
  Serial.begin(57600);

  delay(5000); // give time to physically attach grideye to arduino before starting program
  Serial.println("\nDelay end");
  
  // Get Average Temperature for each Pixel for 1 minute (store in global variable)
  /*for( int i = 0; i < 60; i++ ) {
    delay(1000);
  }*/

  GE_GridEyeSensor.init(0);
}

void loop() {
  // put your main code here, to run repeatedly:

  // Get Temperature
  float thermistorTemp; 
  float pixelsTemp[SNR_SZ];
  getTempArray(&thermistorTemp, pixelsTemp);
  
  // Print to Serial
  Serial.print("*** ");
  Serial.print(thermistorTemp);
  Serial.print(" | ");
  for( int i = 0; i < SNR_SZ; i++ ) {
    Serial.print(pixelsTemp[i]);
    Serial.print(" ");
  }
  Serial.print("\r\n");

  // Check each pixel whether it is above temperature threshold 

  // Count no. of pixels above temperature threshold, if it is above 'lift is full' threshold

  // Send data to Raspberry Pi (Amir/Sam's code)

  // Main delay (update frequency)
  delay(1000);
}
