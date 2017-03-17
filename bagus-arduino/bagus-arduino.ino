#include <Wire.h>
#include "Arduino.h"
#include "grideye.h"
#include "Send.h"
#include <SPI.h>
#include <WiFi.h>
#include <WiFiUdp.h>

/*******************************************************************************
  variable value definition
*******************************************************************************/
grideye  GE_GridEyeSensor;

int status = WL_IDLE_STATUS;
char ssid[] = "pi"; //  your network SSID (name)
char pass[] = "amirdhada";    // your network password (use for WPA, or use as key for WEP)

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

void serialPrint(float thermistorTemp, float *pixelsTemp) {
  Serial.print(thermistorTemp);
  Serial.print(" | ");
  for( int i = 0; i < SNR_SZ; i++ ) {
    Serial.print(pixelsTemp[i]);
    Serial.print(" ");
  }
  Serial.print("\r\n");
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
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid,pass);
    Serial.println(status);

    // wait 10 seconds for connection:
    delay(5000);
  }
  Serial.println("Connected to wifi");
  //printWifiStatus();
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
  serialPrint(thermistorTemp, pixelsTemp);
  sendPacket(pixelsTemp);
  // Check each pixel whether it is above temperature threshold 

  // Count no. of pixels above temperature threshold, if it is above 'lift is full' threshold

  // Send data to Raspberry Pi (Amir/Sam's code)

  // Main delay (update frequency)
  delay(1000);
}
