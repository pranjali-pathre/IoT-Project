#include "DHT.h"

int led1=5;//First batch of LEDs
int led2=4;//Second batch of LEDs
int photo = A0;//Analog reading of photo resistor (LDR) for light intensity
int present = 1;//The present batch being used - odd if first batch is used, even if second is used
int d1 = 0;//pin for first batch DHT
int d2 = 2;//pin for second batch DHT

DHT dht1(d1,DHT11);
DHT dht2(d2,DHT11);

void setup() 
{
  dht1.begin();
  dht2.begin();
  Serial.begin(9600);
  pinMode(led1,OUTPUT);
  pinMode(led2,OUTPUT);
}

void loop() 
{
  int value = analogRead(photo);//Reading the value from LDR
  float inten = (value*5.0)/1023.0;//Converting to voltage (0-5V)
  Serial.println("The present intensity is: ");
  Serial.print(inten);
  if(inten < 2.5)//If intensity is less than half of maximum
  {
    turn_on();//Turn on the right batch
  }
  else
  {
    //turn off both batches as intensity is already high enough
    digitalWrite(led1,LOW);
    digitalWrite(led2,LOW);
  }
}

void turn_on()
{
  float temp1 = dht1.readTemperature();//attached to first batch
  float temp2 = dht2.readTemperature();//attached to second batch
  if(present%2 == 0)
  {
    if(temp2>30)//If the second batch has heated up
    {
      //Glow both batches for 10seconds to avoid sudden loss of light
      digitalWrite(led1,HIGH);
      digitalWrite(led2,HIGH);
      delay(10000);
      //Switch to furst batch
      digitalWrite(led1,HIGH);
      digitalWrite(led2,LOW);
      present++;
    }
  }
  if(present%2 == 1)
  {
    if(temp1>30)//If the first batch has heated up
    {
      //Glow both batches for 10seconds to avoid sudden loss of light
      digitalWrite(led1,HIGH);
      digitalWrite(led2,HIGH);
      delay(10000);
      //Switch to second batch
      digitalWrite(led2,HIGH);
      digitalWrite(led1,LOW);
      present++;
    }
  }
}
