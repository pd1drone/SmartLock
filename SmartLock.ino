#include <SoftwareSerial.h>
#include <Arduino.h>
#include<TinyGPS.h>
TinyGPS gps;
//SoftwareSerial Serial1(6, 5); //tx & rx is connected to Arduino #6 & #5 RX PIN ARDUINO TO tx of GPS and TX PIN OR ARDUINO TO tx of GPS
SoftwareSerial sim(10, 9); //sim Tx & Rx is connected to Arduino #10 & #9

const int buzzer = 4; //buzzer to arduino pin 9
const int GPSbutton = 7; //GPSbutton to arduino pin 7
const int BuzzeroffBtn = 8; //BuzzeroffBtn to arduino pin 8
String number = "+639265556648"; //-> change with your number
String _buffer; // buffer string for SMS text
String SmsString;

// initialize variables for Getdistance function for calculation if the bike has been moved
float Distance;
float gpslat, gpslon;
float latt1, long1;

bool isBuzzerOn = false;

String GoogleMapLink = "https://www.google.com.ph/maps/place/";

bool IsGPSTrackingon = false;

bool initializeGPS = false;
void setup() {
  delay(7000);
  // put your setup code here, to run once:
  _buffer.reserve(200);
  Serial.begin(9600);
  sim.begin(9600);
  Serial1.begin(9600); // connect gps sensor
  delay(5000);
  RecieveMessage();
  delay(5000);
  pinMode(buzzer, OUTPUT); // Set buzzer - pin 4 as an output
  pinMode(GPSbutton, INPUT_PULLUP); // Set GPSbutton - pin 7 as an INPUT_PULLUP
  pinMode(BuzzeroffBtn, INPUT_PULLUP); // Set BuzzeroffBtn - pin 8 as an INPUT_PULLUP
  SendMessage("Smart-Bike-Lock");
}

void loop() {
  // put your main code here, to run repeatedly:
  // Wait for Text Message
  //GetCoordinates();
  //Serial1.listen();
  while (Serial1.available())
      {
        if (gps.encode(Serial1.read()))
        {
          gps.f_get_position(&gpslat, &gpslon);
          latt1= gpslat;
          long1= gpslon;          
          delay(1000);
          Serial.print("LAT 1: ");        
          Serial.println(latt1, 6);
          Serial.print("LON 1: ");
          Serial.println(long1, 6);
        }
      }
      
  GetSMSText();

  // read if buzzer pushbtn has been pushed then set turn-off buzzer
  int Buzzeroff = digitalRead(BuzzeroffBtn);
  Serial.print("BuzzerButton: ");
  Serial.println(Buzzeroff);
  if (Buzzeroff == 0) {
    String Message = "Alarm has been turned OFF";
    SendMessage(Message);
    noTone(buzzer);
  }

  //Check if isLockBypassed Then Notify the lock has been bypassed
  int isLockByPassed = analogRead(A1);
  Serial.print("LockBypass: ");
  Serial.println(isLockByPassed);
  if (!isBuzzerOn){
      if (isLockByPassed > 100) {
      SendMessage("The lock has been bypass!");
      tone(buzzer, 1000);
      isBuzzerOn = true;
    }
  }
}


void RecieveMessage()
{
  //sim.listen();
  Serial.println ("SIM800L Read an SMS");
  delay (1000);
  sim.println("AT+CNMI=2,2,0,0,0"); // AT Command to receive a live SMS
  delay(1000);
  Serial.write ("Unread Message done");
}

String _readSerial() {
  //sim.listen();
  if (sim.available() > 0) {
    return sim.readString();
  }
}

void GetSMSText() {
  //sim.listen();
  if (sim.available() > 0) {
    SmsString = sim.readString();
    Serial.println(SmsString);
    if (SmsString.indexOf("+CMT:") > 0) {
      if (SmsString.indexOf("Send current") > 0) {
        String Link = "GoogleMap Link: " + GoogleMapLink + String(latt1,6) + "," + String(long1,6);
        String SendCurrLocation = "The current Location of your bike is: Latitude: " + String(latt1,6) + " Longitude: " + String(long1,6) + "\n" + Link;
        SendMessage(SendCurrLocation);
      }
    }
  }
}

//Send Message funtion that will send SMS message when called
void SendMessage(String in)
{
  //Serial.println ("Sending Message");
  sim.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
  delay(1000);
  //Serial.println ("Set SMS Number");
  sim.println("AT+CMGS=\"" + number + "\"\r"); //Mobile phone number to send message
  delay(1000);
  String SMS = in;
  sim.println(SMS);
  delay(100);
  sim.println((char)26);// ASCII code of CTRL+Z
  delay(1000);
  _buffer = _readSerial();
}
