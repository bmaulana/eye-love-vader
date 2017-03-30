#include <SPI.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <string.h>

WiFiUDP Udp;
void int2char(float num, char *hello){
  num *= 100;
  int test = num/1000;
  num -= test*1000;
  //Serial.println(test);
  hello[0]=test+'0';
  test = num/100;
  num -= test*100;
  hello[1]=test+'0';
  hello[2] = '.';
  test = num/10;
  num -= test*10;
  hello[3]=test+'0';
  hello[4] = num+'0';
  hello[5]=',';
  hello[6]='\0';
  //Serial.print(hello);
  //return "boo";
}

void float2string(char *x,float *y){
  for (int a=0;a<64;a++){
    char hello[6];
    int2char(y[a],hello);
    strcat(x,hello);
  }
 Serial.println(x);
}

void sendPacket(float *y){
    char string[384];
    //strcat(string,"test ");
    float2string(string,y);
    Serial.println(string);
    Udp.beginPacket("10.0.0.1", 80);
    Udp.write(string, 384);
    Udp.endPacket();
    Serial.println();
}



