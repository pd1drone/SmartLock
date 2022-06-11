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
float latt2;
float long2;

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
          if(latt1 ==0.0 && long2 == 0.0){
            latt1= gpslat;
            long1= gpslon;          
          } else {
            latt2= gpslat;
            long2= gpslon;
          }
  
          delay(1000);
          Serial.print("LAT 1: ");        
          Serial.println(latt1, 6);
          Serial.print("LON 1: ");
          Serial.println(long1, 6);
          Serial.print("LAT 2: ");
          Serial.println(latt2, 6);
          Serial.print("LON 2: ");
          Serial.println(long2, 6);
        }
      }
   if( latt1 != 0.0 || latt2 != 0.0 || long1 != 0.0 || long2 !=0.0){
    Distance = Getdistance(latt1, latt2, long1, long2);
    Serial.print("DISTANCE: ");
    Serial.println(Distance);
   }
     
  GetSMSText();
  // read if gps pushbtn has been pushed then set GPS tracking on
  int GPSbtnOn = digitalRead(GPSbutton);
  Serial.print("GPS BUTTON: ");
  Serial.println(GPSbtnOn);
  if (GPSbtnOn == 0) {
    //Set IsGPSTrackingon to true
    String Message = "GPS Tracking ON";
    SendMessage(Message);
    IsGPSTrackingon = true;
  }

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

  //Check if offGPSTracking Then set IsGPSTrackingon to false and reset lattitude and longitude values
  int offGPSTracking = analogRead(A2);
  Serial.print("GPS TRACKING OFF: ");
  Serial.println(offGPSTracking);
  if (offGPSTracking > 50) {
    String Message = "GPS Tracking is now OFF";
    SendMessage(Message);
    IsGPSTrackingon = false;
    latt1 = 0.000000;
    long1 = 0.000000;
    latt2 = 0.000000;
    long2 = 0.000000;

  }
  // Check if IsGPSTrackingon is true then TrackGPSCoordinates and compute distance
  if (IsGPSTrackingon) {
    // Check if bike has been moved if true then Notify the coordinates of the bike every 10 seconds
    if (Distance > 50) {
        String Link = GoogleMapLink + String(latt2,6) + "," + String(long2,6);
        String DistanceMessage = "Current Coordinates: Lat: " + String(latt2,6) + " Long: " + String(long2,6) + "\n Google Map Link: " + Link;
        String Message = "Your bike was moved from another location";
        SendMessage(Message);
        delay(1000);
        SendMessage(DistanceMessage);
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
        String Link = "GoogleMap Link: " + GoogleMapLink + String(latt2,6) + "," + String(long2,6);
        String SendCurrLocation = "The current Location of your bike is: Latitude: " + String(latt2,6) + " Longitude: " + String(long2,6) + "\n" + Link;
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

float Getdistance(float lat1, float lat2, float lon1, float lon2) {

  // Convert Degree to Radians
  lon1 = lon1 * PI / 180;
  lon2 = lon2 * PI / 180;
  lat1 = lat1 * PI / 180;
  lat2 = lat2 * PI / 180;

  // Haversine formula
  float dlon = lon2 - lon1;
  float dlat = lat2 - lat1;
  float a = pow(sin(dlat / 2), 2) + cos(lat1) * cos(lat2) * pow(sin(dlon / 2), 2);

  float c = 2 * asin(sqrt(a));

  // Radius of earth in kilometers.
  float r = 6371;

  // calculate the result
  return (c * r * 1000); //result unit is meters
}
