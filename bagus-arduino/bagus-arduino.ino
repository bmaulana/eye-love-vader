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
void GE_SourceDataInitialize( short *data ) {
  for ( int i = 0; i < SNR_SZ; i++ ) {
    data[i] = 0;
  }
}

void GE_SourceDataInitializeF( float *data ) {
  for ( int i = 0; i < SNR_SZ; i++ ) {
    data[i] = 0.0;
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

  GE_GridEyeSensor.init(0);
}

void loop() {
  // put your main code here, to run repeatedly:
  
  // Get thermistor register value
  UCHAR aucThsBuf[GRIDEYE_REGSZ_THS];
  GE_GridEyeSensor.bAMG_PUB_I2C_Read(GRIDEYE_REG_THS00, GRIDEYE_REGSZ_THS, aucThsBuf);

  // Get temperature register value
  UCHAR aucTmpBuf[GRIDEYE_REGSZ_TMP];
  GE_GridEyeSensor.bAMG_PUB_I2C_Read(GRIDEYE_REG_TMP00, GRIDEYE_REGSZ_TMP, aucTmpBuf);

  // Convert raw values to short
  short g_shThsTemp = shAMG_PUB_TMP_ConvThermistor(aucThsBuf);

  short g_ashRawTemp[SNR_SZ];
  GE_SourceDataInitialize(g_ashRawTemp);
  vAMG_PUB_TMP_ConvTemperature64(aucTmpBuf, g_ashRawTemp);
  
  // Convert short values to float
  float thermistorTemp = fAMG_PUB_CMN_ConvStoF(g_shThsTemp);
  
  float pixelsTemp[SNR_SZ];
  GE_SourceDataInitializeF(pixelsTemp);
  for ( int i = 0; i < SNR_SZ; i++ ) {
    pixelsTemp[i] = fAMG_PUB_CMN_ConvStoF(g_ashRawTemp[i]);
  }
  
  // Print to Serial
  Serial.print("*** ");
  Serial.print(thermistorTemp);
  Serial.print(" ");
  for( int i = 0; i < SNR_SZ; i++ ) {
    Serial.print(pixelsTemp[i]);
    Serial.print(" ");
  }
  Serial.print("\r\n");

  // Main delay (update frequency)
  delay(1000);
}
