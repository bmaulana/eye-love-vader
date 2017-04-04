/*
  Analog Input
 Demonstrates analog input by reading an analog sensor on analog pin 0 and
 turning on and off a light emitting diode(LED)  connected to digital pin 13.
 The amount of time the LED will be on and off depends on
 the value obtained by analogRead().

 The circuit:
 * Potentiometer attached to analog input 0
 * center pin of the potentiometer to the analog pin
 * one side pin (either one) to ground
 * the other side pin to +5V
 * LED anode (long leg) attached to digital output 13
 * LED cathode (short leg) attached to ground

 * Note: because most Arduinos have a built-in LED attached
 to pin 13 on the board, the LED is optional.


 Created by David Cuartielles
 modified 30 Aug 2011
 By Tom Igoe

 This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/AnalogInput

 */
// constants (replace with #define??)
#define sensorPin A8
#define movingAvgLen 10
#define threshold 4

// globals
int movingAvg[movingAvgLen] = {0};
int toSend = 0;

void setup() {
  Serial.begin(57600);

  //TODO connect to Pi using Amir's code
  
  // wait until it stabilises
  while(true){
    int avg = 0;
    for(int i = 0; i < 5; i++){
      avg += analogRead(sensorPin);
      delay(100);
    }
    avg = avg/5;
    Serial.print(avg);

    int movingAvgVal = 0;
    for(int i = 0; i < movingAvgLen; i++){
      movingAvgVal += movingAvg[i]; 
    }
    movingAvgVal = movingAvgVal / movingAvgLen;
    Serial.print(" ");
    Serial.println(movingAvgVal);

    for(int i = 0; i < movingAvgLen - 1; i++){
      movingAvg[i] = movingAvg[i+1];
    }
    movingAvg[movingAvgLen - 1] = avg;
    
    if((avg - movingAvgVal) >= -1 && (avg - movingAvgVal) <= 1) {
      break;
    }
  }

  Serial.println("Sensor stabilised");
}

void loop() {
  // read the value from the sensor:
  int avg = 0;
  for(int i = 0; i < 5; i++){
    avg += analogRead(sensorPin);
    delay(100);
  }
  avg = avg/5;
  Serial.print(avg);
  
  int movingAvgVal = 0;
  for(int i = 0; i < movingAvgLen; i++){
    movingAvgVal += movingAvg[i];
  }
  movingAvgVal = movingAvgVal / movingAvgLen;
  Serial.print(" ");
  Serial.println(movingAvgVal);

  if(toSend == 0 && avg >= movingAvgVal + threshold) {
    Serial.println("Motion detected");
    toSend = 1;
  } else if(toSend == 1 && avg <= movingAvgVal - threshold) {
    Serial.println("Motion stopped");
    toSend = 0;
  }
 
  // TODO send data over to Raspberry Pi using Amir's code
  

  for(int i = 0; i < movingAvgLen; i++){
    movingAvg[i] = movingAvg[i+1];
  }
  movingAvg[movingAvgLen - 1] = avg;
}
