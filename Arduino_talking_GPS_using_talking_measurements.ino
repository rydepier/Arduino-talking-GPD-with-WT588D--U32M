/*------------------------------------------------------------
Talking GPS with WT588D - U 32M audio module
Using audio file 'Talking Measurements'

Uses TinyGPS Plus download library from 
http://arduiniana.org/libraries/tinygpsplus/

Requires Arduino Mega

Connections::

WTD588D::
Upload the audio to this device before connecting into the circuit

Arduino pin 3 to Module pin 7 RESET
Arduino pin 4 to Module pin 21 BUSY
Arduino pin 5 to Module pin 18 PO1
Arduino pin 6 to Module pin 17 PO2
Arduino pin 7 to Module pin 16 PO3
Arduino Gnd to Module pin 14
Arduino 5v to Module pin 20

Push Button::
Connect N/O push switch between
Arduino pin 2
Arduino pin Gnd
(Note pin 2 is held HIGH by the internal pullup resistor)

GPS Module (Arduino pin connections are for Uno or Mega

Gnd connects to Arduino Gnd
Rx connects to Software Serial pin 11 on the Arduino, via logic level shifter if required
Tx connects to Software Serial pin 10 on the Arduino, via logic level shifter if required
Vcc connects to 3.3 volts, or 5 volts as required

GPS phrases::
*****************************************************
*and                                     39
*degree                                  152
*degrees                                 57
*east                                    69
*foot                                    162
*feet                                    163
*gps fix                                 72
*gps location                            73
*gps                                     74
*ground speed is                         75
*kph (kilometers per hour)               86
*latitude                                154
*longitude                               155
*metre                                   156
*metres                                  157
*mile                                    158
*miles                                   159
*minute                                  96
*minutes                                 97
*mph (miles per hour)                    99
*no gps fix                              100
*number of satellites is                 103
*satellites in view                      118
*second                                  153
*seconds                                 119
*south                                   123
*the height is                           130
*west                                    148
*your location is                        150
*your position is                        151

*****************************************************

By Chris Rouse
Feb 2016
------------------------------------------------------------*/
// incluide the Library
#include "WT588D.h"  // audio module
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
// The TinyGPS++ object
TinyGPSPlus gps;
// The serial connection to the GPS device
static const int RXPin = 10, TXPin = 11;
static const uint32_t GPSBaud = 9600;
SoftwareSerial ss(RXPin, TXPin);
//
// define pins etc
//
// set the correct pin connections for the WT588D chip
#define WT588D_RST 3  //Module pin "REST"
#define WT588D_CS 6   //Module pin "P02"
#define WT588D_SCL 7  //Module pin "P03"
#define WT588D_SDA 5  //Module pin "P01"
#define WT588D_BUSY 4 //Module pin "LED/BUSY"  
WT588D myWT588D(WT588D_RST, WT588D_CS, WT588D_SCL, WT588D_SDA, WT588D_BUSY);
#define talkPin 2 // pin used to request speach
#define ledPin 13 // onboard LED
//
int tensOffset = 0x12; // 2 less than its actual value
int hundredsOffset = 0x1b; // 1 less than its actual value
boolean andPhrase = false; // shows if and phrase is needed
boolean minus = false; // used to show when minus must be added to a number
boolean northFlag = true; // true if latitude value is positive
boolean westFlag = true; // true if latitude is negative
double extracted; // used in speakNumber
int temp; // gp variable
float minSec; // used in speak GPS location
float temp2; // gp variable
float longLatValue; // decimal value for latitude or longitude
/*--------------------------------------------------------*/

void setup() {
  ss.begin(GPSBaud);
  Serial.begin(115200); // used for debug if required
  pinMode(talkPin, INPUT_PULLUP); // saves having to use an external resistor
  pinMode(ledPin, OUTPUT); // onboard LED
  digitalWrite(ledPin, LOW); // turn off onboard LED
  //
  // initialize the chip and port mapping
  myWT588D.begin();
  //  
  // play boot up sound
  speakPhrase(116); // ready
  delay(50); // short delay
  speakPhrase(74); // GPS
}
/*--------------------------------------------------------*/
void loop() {
  if(digitalRead(talkPin) == 0 ){ // talk key pressed
    if (gps.location.isValid()){ // check to see if a gps fix is available
      speakGPS(); // if it is then talk the location
    }
    else speakNoGPS(); // otherwise report **no fix**
  } 
  //
  //ensures data is updated on a regular basis
  smartDelay(1000);
  //
  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    speakPhrase(101); // no
    speakPhrase(74); // gps
    speakPhrase(43); // can be detected
    while(true); // wait here
  }  
}
/*--------------------------------------------------------*/
// This custom version of delay() ensures that the gps object
// is being "fed".
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}
/*--------------------------------------------------------*/
// Talking Measurements
/*--------------------------------------------------------*/
void speakGPS(){
  // a valid GPS fix has been obtained
  speakPhrase(150); // your location is
  speakPhrase(154); // latitude
  longLatValue = (gps.location.lat());
  if(longLatValue <0){
    northFlag = false;
    longLatValue = longLatValue * -1;
  }
  speakOutput();
  if(northFlag) speakPhrase(102); // north
  else speakPhrase(123); // south
  delay(500);
  // now do longitude
  speakPhrase(155); // longitude
  longLatValue = (gps.location.lng());
  if(longLatValue <0){
    westFlag = false;
    longLatValue = longLatValue * -1;
  }
  speakOutput();  
  if(westFlag) speakPhrase(69); // east
  else speakPhrase(148); // west
  //
  // now tell us how many satellites are in view
  delay(500);
  speakPhrase(138); // there are
  speakNumber(gps.satellites.value());
  speakPhrase(118); // satellites in view+
  delay(500);
  // altitude
  speakPhrase(130); // the height is
  temp = gps.altitude.meters();
  speakNumber(temp);
  speakPhrase(157); // metres
}
/*--------------------------------------------------------*/
void speakOutput(){
  temp = longLatValue;
  minSec = longLatValue-temp;
  if(temp <1){
    temp = temp * -1;
    northFlag = false;
  }
  else northFlag = true;
  speakNumber(temp);
  if(temp == 1){
    speakPhrase(152); // degree
  }
  else speakPhrase(57); // degrees
  temp = (minSec *6000000)/100000;  // get minutes
  speakNumber(temp);
  if(temp == 1){
    speakPhrase(96); // minute
  }
  else speakPhrase(97); // minutes
  temp2 = ((minSec *6000000)/100000) - temp;
  temp = 60*temp2; // get seconds
  speakPhrase(39); // and
  speakNumber(temp);
  if(temp == 1){
    speakPhrase(153); // second
  }
  else speakPhrase(119); // seconds  
}
/*--------------------------------------------------------*/
void speakNoGPS(){
  // no valid GPS fix has been obtained
  speakPhrase(100); // no GPS fix
}
/*--------------------------------------------------------*/
void busy(int pause){
  // waits for WT588D to finish sound
  delay(100);
  while (myWT588D.isBusy() ) {
  }
  delay(pause);
}
//
void speakPhrase(int phrase) {
  myWT588D.playSound(phrase);
  busy(0);
}
//
/*-----------------------------------------------------*/
void speakNumber(int number){
  // will speak an integer from 1 to 9999
  // speak a number routine
  //
  if(number > 99){
   andPhrase = true;
  }
  else andPhrase = false;
  //
  if(minus) speakPhrase(0x5F); // minus
   // positive number over 1000
  if(number >= 1000){ 
    extracted = number/1000; // adjust value
    speakPhrase(extracted);
    speakPhrase(0x8B); // thousand
    number = number - extracted*1000;
  }
  // positive number from 100 to 999
  if(number >=100){
    extracted = number/100;
    speakPhrase(extracted+ hundredsOffset);
    number = number - extracted*100;
    if(number > 0){
      speakPhrase(0x27); // short and
      andPhrase = false;
    }
  }
  // decade from 20 to 90
  if(number>=20){
    if(andPhrase ){
      speakPhrase(0x27); // short and
      andPhrase = false;
    }
    extracted = number/10;  
    speakPhrase(extracted + (tensOffset));
    number = number - extracted*10;
  }
  // below  20
  if(number>0){
    if(andPhrase){
      speakPhrase(0x27); // short and
    }
    speakPhrase(number);
  }
}
/*--------------------------------------------------------*/
void speakDecimal(float number){
  // speaks 1 decimal place 
  speakPhrase(0x72); // point
  extracted = (10 * number) - (10 * int(number));  
  speakPhrase(extracted);
}
/*--------------------------------------------------------*/
