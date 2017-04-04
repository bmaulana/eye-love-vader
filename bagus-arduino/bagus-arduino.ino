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
#define   MAIN_DELAY        1000
#define   INIT_TIME         30
#define   TEMP_THRESHOLD    5.0
#define   PIXELS_THRESHOLD  32

grideye   GE_GridEyeSensor;
float     averageTemp[SNR_SZ]; 

int status = WL_IDLE_STATUS;
char ssid[] = "pi"; //  your network SSID (name)
char pass[] = "amirdhada";    // your network password (use for WPA, or use as key for WEP)

/*******************************************************************************
  methods
 ******************************************************************************/
void initShort(short *data) {
  for ( int i = 0; i < SNR_SZ; i++ ) {
    data[i] = 0;
  }
}

void initFloat(float *data) {
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
  initShort(g_ashRawTemp);
  vAMG_PUB_TMP_ConvTemperature64(aucTmpBuf, g_ashRawTemp);
  
  // Convert short values to float
  *thermistorTemp = fAMG_PUB_CMN_ConvStoF(g_shThsTemp);
  
  initFloat(pixelsTemp);
  for (int i = 0; i < SNR_SZ; i++) {
    pixelsTemp[i] = fAMG_PUB_CMN_ConvStoF(g_ashRawTemp[i]);
  }
}

void serialPrint(float thermistorTemp, float *pixelsTemp) {
  Serial.print(thermistorTemp);
  Serial.print(" | ");
  for(int i = 0; i < SNR_SZ; i++) {
    Serial.print(pixelsTemp[i]);
    Serial.print(" ");
  }
  Serial.print("\r\n");
}

void serialPrintBool(int ctr, bool *arr) {
  Serial.print(ctr);
  Serial.print(" | ");
  for(int i = 0; i < SNR_SZ; i++) {
    Serial.print(arr[i]);
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
    delay(10000);
  }
  Serial.println("Connected to wifi");
  //printWifiStatus();

  GE_GridEyeSensor.init(0);

  initFloat(averageTemp);
  Serial.println("Initial average temps (should all be 0.0)");
  serialPrint(0.0, averageTemp);

  Serial.println("Starting average temperature initialisation");
  
  // Get Average Temperature for each Pixel for 1 minute (store in global variable)
  for(int i = 0; i < INIT_TIME; i++) {
    float thermistorTemp; 
    float pixelsTemp[SNR_SZ];
    getTempArray(&thermistorTemp, pixelsTemp);
    serialPrint(thermistorTemp, pixelsTemp);
  
    for(int j = 0; j < SNR_SZ; j++) {
      averageTemp[j] = averageTemp[j] + pixelsTemp[j]; 
    }
    
    delay(MAIN_DELAY);
  }

  for(int i = 0; i < SNR_SZ; i++) {
    averageTemp[i] = averageTemp[i] / INIT_TIME;
  }

  Serial.println("Average temperature initialised");
  serialPrint(0.0, averageTemp);
  Serial.println("Running in regular mode");
}

void loop() {
  // put your main code here, to run repeatedly:

  // Get Temperature
  float thermistorTemp; 
  float pixelsTemp[SNR_SZ];
  getTempArray(&thermistorTemp, pixelsTemp);
  serialPrint(thermistorTemp, pixelsTemp);
  
  // Check each pixel whether it is above temperature threshold
  boolean pixelsThreshold[SNR_SZ];
  int aboveThreshold = 0;
  for(int i = 0; i < SNR_SZ; i++) {
    if(pixelsTemp[i] > averageTemp[i] + TEMP_THRESHOLD) {
      pixelsThreshold[i] = true;
      aboveThreshold++;
    } else {
      pixelsThreshold[i] = false;
    }
  }
  serialPrintBool(aboveThreshold, pixelsThreshold);

  if(aboveThreshold >= PIXELS_THRESHOLD) {
    Serial.println("Lift is full");
  }
  
  // Send data to Raspberry Pi (Amir/Sam's code)
  sendPacketInt(aboveThreshold, 80);

  // Main delay (update frequency)
  delay(MAIN_DELAY);
  // TODO poll 10x per second and use rolling average of temperature in main loop
}

