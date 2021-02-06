#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SoftwareSerial.h>
#include <Servo.h>

char auth[] = "AMYbwPfEK-ljR66AYt7CZMfNP-o6DbTG";
char ssid[] = "saketharama";
char pass[] = "veenrrr2010a";
WiFiClient client;

Servo servo;

/*
   Virtual pins on Blynk app are:
   V2 - Text box for entering the OTP obtained
   V3 - Open button
   V4 - Close button
   V5 - Text box for showing the OTP (Showed in notifications too)
   V7 - LED for PIR sensor output
   V8 - LED for IR sensor output
   V9 - Text box for showing intruder related messages
*/

int pir_s = 0;//PIR sensor
int ir_s = 5;//IR sensor
int LED = 4;
int door_open = 2;//Pin to open the door
int door_close = 14;//Pin to close the door
int OTP = 0;//The OTP the user enters to authenticate
int state_open = 0;//The state of the door, if it is open or not
int state_close = 0;//The state of the door, if it is close or not
int tries = 5;//Number of tries left with the user at an instant to be allowed to open the door using Blynk app
int now = 0;//Used for giving the user a 1 minute cooldown before he attempts to open again
int wait = 0;//Flag whether to wait or not

void setup()
{
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);//Start Blynk app
  servo.attach(door_open);
  pinMode(pir_s, INPUT_PULLUP);//PIR sensor
  pinMode(ir_s, INPUT);//IR sensor
  pinMode(door_open, OUTPUT);//Door open
  pinMode(door_close, OUTPUT);//Door closed
  digitalWrite(door_open, LOW);//Initially set to no operation, keep it in its present state
  digitalWrite(door_close, LOW);//Initially set to no operation, keep it in its present state
  digitalWrite(pir_s,LOW);
  digitalWrite(ir_s, LOW);
}

void loop()
{
  WiFi.begin(ssid, pass); //Start WiFi connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
  }
  Serial.println("CONNECTED");
  Blynk.run();//Run Blynk app
  sensorvalue1();
  delay(3000);
}


BLYNK_WRITE(V3)//For open door button
{
  int pinValue = param.asInt();
  state_open = pinValue;
  if (pinValue)
  {
    if (state_close) //If both open and close buttons are on
    {
      Blynk.notify("Please turn off the close button, and retry by turning off and again turning on this button");
    }
    else
    {
      int x = random(9999);
      OTP = x;
      Blynk.virtualWrite(V5, x);
      String op = (String) x;
      op = op + " is the code for authorisation. Enter this in the text area beside for authentication";
      Blynk.notify(op);
    }
  }
  else
  {
    digitalWrite(door_open, LOW);
    Blynk.notify("door opening stopped");
  }
}

BLYNK_WRITE(V4)//For close door button
{
  int pinValue = param.asInt();
  state_close = pinValue;

  if (pinValue)
  {
    if (state_open) //If both open and close buttons are on
    {
      Blynk.notify("Please turn off the open button, and retry by turning off and again turning on this button");
    }
    else
    {
      servo.write(-180);
      delay(1000);
      Blynk.notify("Closing door");
    }
  }
  else
  {
    Blynk.notify("Stopped closing door");
  }
}

BLYNK_WRITE(V2)//For authorisation
{
  int rn = millis();
  if (wait && rn - now < 60000) //During cooldown time
  {
    int left = 60000 - (rn - now);
    left /= 1000;
    String op = "Please wait for ";
    op = op + (String) left;
    op = op + " more seconds before you try to unlock the door";
    Blynk.notify(op);
  }
  else
  {
    tries--;
    if (tries == 0) //Out of tries
    {
      Blynk.notify("The door cannot be opened using the app now. Please retry after 1 minute");
      now = millis();
      wait = 1;
      tries = 5;
    }
    else
    {
      wait = 0;
      int pinValue = param.asInt();
      Serial.println(pinValue);
      if (OTP == pinValue) //Matched
      {
        servo.write(180);
        delay(1000);
        Blynk.notify("door opened successfully");
        tries = 5;
      }
      else
      {
        //Wrong OTP entered
        if (tries != 1)
        {
          String op = "Wrong OTP entered, You have ";
          op = op + (String) tries;
          op = op + " more tries left";
          Blynk.notify(op);
        }
        else
        {
          Blynk.notify("Wrong OTP entered, You have just 1 more try left!!");
        }
      }
    }
  }
}

void sensorvalue1()//For intruder alert
{
  if(digitalRead(pir_s) == HIGH)//PIR detects
  {
    digitalWrite(LED, HIGH);
    Blynk.virtualWrite(V7, 255);
    Blynk.virtualWrite(V9, "Intruder detected on PIR Sensor!!!");
    Blynk.notify("Intruder detected on PIR Sensor!!!");
    delay(1000);
  }

  if(digitalRead(pir_s) == LOW)//PIR doesn't detect
  {
    Blynk.virtualWrite(V7, 0);
    Blynk.virtualWrite(V9, "PIR Normal");
    delay(1000);
  }

  if ( digitalRead(ir_s) == LOW) //IR detects
  {
    Blynk.virtualWrite(V8, 255);
    Blynk.virtualWrite(V9, "Intruder detected on IR Sensor!!!");
    Blynk.notify("Intruder detected on IR Sensor!!!");
    delay(1000);
  }

  if ( digitalRead(ir_s) == HIGH) //IR doesn't detect
  {
    Blynk.virtualWrite(V8, 0);
    delay(1000);
  }
}
