
/*
 Elevator Simulation Program
 Created 28 Mar. 2023
 by Asher Linkowski, Marina
 */

#include <Stepper.h>            // Stepper library
#include <SoftwareSerial.h>     // Serial Software library
#include <LiquidCrystal.h>      // include the library code: The LiquidCrystal

#include "ThingSpeak.h"         // Thingspeak library
#include <SPI.h>
#include <WiFiNINA.h>
#include "secrets.h"

char ssid[] = SECRET_SSID;      // Thingspeak wifi access
char pass[] = SECRET_PASS;
WiFiClient theClient;

unsigned long myChannelNumber = SECRET_CH_ID;         // Thingspeak vars
const char myWriteAPIKey[] = SECRET_WRITE_APIKEY;

const int stepsPerRevolution = 500;  // Steps per level
const int torquediff = 1.2;

Stepper myStepper(stepsPerRevolution, 8, 9, 10, 11);    // initialize the stepper library on pins 8 through 11:

int level = 1;    // set starting floor at 1

const int buttonPin1 = 6;     // the number of the pushbutton pin 1
const int buttonPin2 = 7;     // the number of the pushbutton pin 2
int buttonState1 = 0;         // variable for reading the pushbutton 1 status
int buttonState2 = 0;         // variable for reading the pushbutton 2 status
const int ledPin1 =  13;      // the number of the LED 1 pin
const int ledPin2 =  12;      // the number of the LED 2 pin
const int rs = A0, en = A1, d4 = A2, d5 = A3, d6 = A4, d7 = A5; // LCD pin map
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);                      // LCD pin map

SoftwareSerial commModule(2, 3); // RX | TX

byte U1[] = {     // Custom LED byte for the first half of U, for sideways font
  0b00000,
  0b11111,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b11111
};

byte U2[] = {
  0b00000,
  0b11100,
  0b00010,
  0b00001,
  0b00001,
  0b00001,
  0b00010,
  0b11100
};

byte P1[] = {
  0b01110,
  0b10001,
  0b10001,
  0b10001,
  0b10001,
  0b10001,
  0b11111,
  0b00000
};

byte D1[] = {
  0b00000,
  0b00111,
  0b01000,
  0b10000,
  0b10000,
  0b10000,
  0b11111,
  0b00000
};

byte D2[] = {
  0b00000,
  0b11100,
  0b00010,
  0b00001,
  0b00001,
  0b00001,
  0b11111,
  0b00000
};

byte N1[] = {
  0b00000,
  0b11111,
  0b00000,
  0b00000,
  0b00011,
  0b01100,
  0b11111,
  0b00000
};

byte N2[] = {
  0b00000,
  0b11111,
  0b00110,
  0b11000,
  0b00000,
  0b00000,
  0b11111,
  0b00000
};

byte Ar2[] = {
  0b00000,
  0b00000,
  0b10000,
  0b01000,
  0b00100,
  0b00010,
  0b00001,
  0b00000
};

void setup() {

  pinMode(4, OUTPUT);         // intialize pins for BT control
  //digitalWrite(4, HIGH);    // Pin 4 for EN, setup only
  
  myStepper.setSpeed(25);     // set the speed rpm
  Serial.begin(9600);         // initialize the serial port
  ThingSpeak.begin(theClient); //Initialize a connection to ThingSpeak

  pinMode(buttonPin1, INPUT); // initialize the pushbutton pin 1 as input
  pinMode(buttonPin2, INPUT); // initialize the pushbutton pin 2 as input
  pinMode(ledPin1, OUTPUT);   // initialize the LED pin 1 as an output
  pinMode(ledPin2, OUTPUT);   // initialize the LED pin 2 as an output
  buttonState1 = LOW;         // initialize the button 1 state as off
  buttonState2 = LOW;         // initialize the button 1 state as off
  digitalWrite(ledPin1, HIGH);    // initialize the floor with light 1 on
  
  commModule.begin(9600);               // Baud Rate for command Mode.
  Serial.println("Elevator Commands");  // Print initializing text
  commModule.println("Elevator Commands");  // Print initializing text to comm module


  lcd.begin(16, 2);         // set up the LCD's number of columns and rows
  lcd.setCursor(0, 0);      // set the cursor to column 0, line 0
  lcd.createChar(0, U1);    // create custom characters from byte maps
  lcd.createChar(1, U2);
  lcd.createChar(2, D1);
  lcd.createChar(3, D2);
  lcd.createChar(4, N1);
  lcd.createChar(5, N2);
  lcd.createChar(6, Ar2);
  lcd.clear();              // Clears the LCD screen:
}

void loop() {



  if(WiFi.status()!=WL_CONNECTED){
    Serial.print("Attempting to connect to SSID:");
    Serial.println(SECRET_SSID);
    while (WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass);
      Serial.print(".");
      delay(500);
      }
    Serial.println("\nConnected.");
  }


  ThingSpeak.setField(1, level);
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (x==200){
   Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updateing channel.");
  }
  delay(1000);




  buttonState1 = digitalRead(buttonPin1);   // read the state of the pushbutton 1 value:
  buttonState2 = digitalRead(buttonPin2);   // read the state of the pushbutton 2 value:

  // Floor 1 button
  if (buttonState1 == HIGH){                  // check if the floor 1 button pressed
    if (level != 1){                          // check if the elevator is not on floor 1
    digitalWrite(ledPin2, LOW);               // turn floor 2 LED off
    lcd.clear();              // Clears the LCD screen:
  lcd.setCursor(1, 1);        // This block prints "up"
  lcd.write(byte(2));
  lcd.setCursor(2, 1);
  lcd.write(byte(3));
  
  lcd.setCursor(1, 0);
  lcd.write(byte(4));
  lcd.setCursor(2, 0);
  lcd.write(byte(5));

  lcd.setCursor(4, 0);      // This block prints the moving arrow
   lcd.print("-----");
   lcd.write(byte(6));
   lcd.setCursor(4, 1);
   lcd.print("-----/");

   myStepper.step(-stepsPerRevolution);       // move motor to floor

   lcd.setCursor(4, 0);
   lcd.print(" -----");
   lcd.write(byte(6));
   lcd.setCursor(4, 1);
   lcd.print(" -----/");

   myStepper.step(-stepsPerRevolution);

   lcd.setCursor(4, 0);
   lcd.print("  -----");
   lcd.write(byte(6));
   lcd.setCursor(4, 1);
   lcd.print("  -----/");

   myStepper.step(-stepsPerRevolution);

   lcd.setCursor(4, 0);
   lcd.print("   -----");
   lcd.write(byte(6));
   lcd.setCursor(4, 1);
   lcd.print("   -----/");

   myStepper.step(-stepsPerRevolution);

   lcd.setCursor(4, 0);
   lcd.print("    -----");
   lcd.write(byte(6));
   lcd.setCursor(4, 1);
   lcd.print("    -----/");

   myStepper.step(-stepsPerRevolution);

   lcd.setCursor(4, 0);
   lcd.print("     -----");
   lcd.write(byte(6));
   lcd.setCursor(4, 1);
   lcd.print("     -----/");

    delay(400);                               // wait for "doors to open"
    digitalWrite(ledPin1, HIGH);              // turn floor 1 LED on
    level = 1;                                // now on level 1

    Serial.print("You are on level ");
    Serial.println(level);
    commModule.println(level);
    
    } else {                                  // if it's already on floor 1, blink
      digitalWrite(ledPin1, LOW);
      delay(100);
      digitalWrite(ledPin1, HIGH);
      delay(100);
      digitalWrite(ledPin1, LOW);
      delay(100);
      digitalWrite(ledPin1, HIGH);
      delay(100);
      digitalWrite(ledPin1, LOW);
      delay(100);
      digitalWrite(ledPin1, HIGH);
    }
    }


  // Floor 2 button
  if (buttonState2 == HIGH){                  // check if the floor 2 button pressed
    if (level != 2){                          // check if the elevator is not on floor 1
    digitalWrite(ledPin1, LOW);               // turn floor 1 LED off
    lcd.clear();              // Clears the LCD screen:

  lcd.setCursor(1, 1);        // This block prints "dn"
  lcd.write(byte(0));
  lcd.setCursor(2, 1);
  lcd.write(byte(1));
  
  lcd.setCursor(1, 0);
  lcd.print("O");
  lcd.setCursor(2, 0);
  lcd.print("_");

   lcd.setCursor(4, 0);
   lcd.print("     /-----   ");
   lcd.setCursor(9, 1);
   lcd.write(byte(6));
   lcd.print("-----   ");

  myStepper.step(stepsPerRevolution*torquediff);       // move motor to floor

   lcd.setCursor(4, 0);
   lcd.print("    /-----   ");
   lcd.setCursor(8, 1);
   lcd.write(byte(6));
   lcd.print("-----   ");

  myStepper.step(stepsPerRevolution*torquediff);

   lcd.setCursor(4, 0);
   lcd.print("   /-----   ");
   lcd.setCursor(7, 1);
   lcd.write(byte(6));
   lcd.print("-----   ");
   
  myStepper.step(stepsPerRevolution*torquediff);

   lcd.setCursor(4, 0);
   lcd.print("  /-----   ");
   lcd.setCursor(6, 1);
   lcd.write(byte(6));
   lcd.print("-----   ");
   
   myStepper.step(stepsPerRevolution*torquediff);

   lcd.setCursor(4, 0);
   lcd.print(" /-----   ");
   lcd.setCursor(5, 1);
   lcd.write(byte(6));
   lcd.print("-----   ");

   myStepper.step(stepsPerRevolution*torquediff);

   lcd.setCursor(4, 0);
   lcd.print("/-----   ");
   lcd.setCursor(4, 1);
   lcd.write(byte(6));
   lcd.print("-----   ");
   
    delay(400);                               // wait for "doors to open"
    digitalWrite(ledPin2, HIGH);              // turn floor 2 LED on
    level = 2;                                // now on level 2

    Serial.print("You are on level ");
    Serial.println(level);
    commModule.println(level);

    
    } else {                                  // if it's already on floor 2, blink
      digitalWrite(ledPin2, LOW);
      delay(100);
      digitalWrite(ledPin2, HIGH);
      delay(100);
      digitalWrite(ledPin2, LOW);
      delay(100);
      digitalWrite(ledPin2, HIGH);
      delay(100);
      digitalWrite(ledPin2, LOW);
      delay(100);
      digitalWrite(ledPin2, HIGH);
    }
    }

    if ((buttonState1 == LOW) && (buttonState2 == LOW)){    // If no button was pressed, and it's on a floor, display the floor number

        if (level == 1){
          lcd.setCursor(0, 0);
          lcd.print("  OOOOOOOOOO      ");
          lcd.setCursor(0, 1);
          lcd.print("   O           ");
        }

        if (level == 2){
          lcd.setCursor(0, 0);
          lcd.print("  OOOOO   O    ");
          lcd.setCursor(0, 1);
          lcd.print("  O   OOOOO      ");
        }
    }

}
