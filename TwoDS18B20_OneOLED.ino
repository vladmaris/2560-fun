//
// FILE: TwoDS18B20_OneOLED.ino
// AUTHOR: Vlad Maris
// VERSION: 0.3.00
// PURPOSE: two pins for two sensors demo, with OLED display on I2C
// DATE: 2022-10-30
//


//
// Sensor shield v5.0 pinout:
//
// Pin0 Hall sensor
// Pin1 burnt ?!
// Pin2 DS18B20 interior
// Pin3 DS18B20 exterior
// Pin4
// Pin5
// Pin6
// Pin7
// Pin8
// Pin9
// Pin10 SDcard CS
// Pin11 SDcard MOSI
// Pin12 SDcard MISO
// Pin13 SDcard SCK
// PinA0 Relay
//

#include <stdio.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_GFX.h>      // Include core graphics library for the display
#include <Adafruit_SSD1306.h>  // Include Adafruit_SSD1306 library to drive the display
#include <SD.h>
#include <SPI.h>  //used for SDcard interface


#define ONE_WIRE_BUS_1 2  // assign pin 2 to interior temperature sensor
#define ONE_WIRE_BUS_2 3  // assign pin 3 to exterior temperature sensor
#define hallPin 0         //assign pin 0 to hall sensor
#define relayPin A0       // assign pin A0 for relay control
#define tempSet 25.0      // assign treshold temperature for relay On/Off
int pinCS = 10;           //

OneWire oneWire_t1(ONE_WIRE_BUS_1);
OneWire oneWire_t2(ONE_WIRE_BUS_2);

DallasTemperature sensor_inhouse(&oneWire_t1);
DallasTemperature sensor_outhouse(&oneWire_t2);

Adafruit_SSD1306 display(128, 64);  // Create display

float T1C, T2C, T1F, T2F, T1K, T2K;  // used to store values read from temperature sensors

float *T1Cptr, *T2Cptr, *T1Fptr, *T2Fptr, *T1Kptr, *T2Kptr;  // used to pass the temperature sensors readings

short displayMode;     // Used to navigate between active functions
short oldDisplayMode;  // Used as backup if the new value is not OK

bool isField;  // used for signaling the presence of a field near Hall sensor
bool isRelay;  // used for signaling the relay state

File myFile;  // the file used to log data on SDcard

void setup(void) {

  delay(100);  // This delay is needed to let the display to initialize

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // Initialize display with the I2C address of 0x3C

  display.clearDisplay();  // Clear the buffer

  cli();  //stop interrupts

  //set timer4 interrupt at 1Hz
  TCCR1A = 0;  // set entire TCCR1A register to 0
  TCCR1B = 0;  // same for TCCR1B
  TCNT1 = 0;   //initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 3906 / 1;  // = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12 and CS10 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  sei();  //allow interrupts

  sensor_inhouse.begin();

  sensor_outhouse.begin();

  Serial.begin(9600);  // set serial comm baudrate

  Serial.println("Two Sensors, One Display");  // Print on serial the title of this program

  pinMode(relayPin, OUTPUT);  // sets the digital pin A0 as output

  pinMode(hallPin, INPUT);  // sets the digital pin 9 as output

  display.setTextColor(WHITE);  // Set color of the text

  display.setRotation(0);  // Set orientation. Goes from 0, 1, 2 or 3

  display.setTextWrap(false);  // By default, long lines of text are set to automatically wrap back to the leftmost column.

  display.setTextSize(0);  // Set text size. We are using a custom font so you should always use the text size of 0

  display.dim(1);  //Set brightness (0 is maximun and 1 is a little dim)

  displayMode = 49;     // Initialize displaymode with 49==0, the value read from serial monitor; used for navigating between active functions
  oldDisplayMode = 49;  // Initialize the oldvalue

  T1Cptr = &T1C;
  T2Cptr = &T2C;
  T1Fptr = &T1F;
  T2Fptr = &T2F;
  T1Kptr = &T1K;
  T2Kptr = &T2K;

  pinMode(pinCS, OUTPUT);  //set for sdcard interface
  digitalWrite(10, HIGH);

  // SDcard Initialization
  if (SD.begin()) {
    Serial.println("SDcard is ready to use.");
  } else {
    Serial.println("SDcard initialization failed");
    return;
  }
}


// this is the main loop that switches between the defined functions, as received from serial port treated with interruption
void loop(void) {
  if (displayMode == 49) {                          //if 1 is read
    myFirstPage();                                  // run first page
  } else if (displayMode == 50) {                   //if 2 is read
    mySecondPage();                                 // run second page
  } else if (displayMode == 51) {                   //if 3 is read
    myThirdPage();                                  //run third page
  } else if (displayMode == 52) {                   //if 4 is read
    myFourthPage();                                 // run fourth page
  } else if (displayMode == 53) {                   //if 5 is read
    myFifthPage();                                  // run fifth page
  } else if (displayMode == 54) {                   //if 6 is read
    mySixthPage();                                  // run sixth page
  } else if (displayMode == 55) {                   //if 7 is read
    mySeventhPage();                                // run seventh page
  } else {                                          //if none
    Serial.println("displayMode uninitialised!!");  // announce no value for page to be displayed; failsafe mechanism
    displayMode = 49;                               // initialise
    myFirstPage();                                  //ensure enter in something
  }
}  //end of loop


// this function reads and displays general sensor readings, RTC, relay
void myFirstPage() {
  //print sensor data when displayMode equals 1
  sensor_inhouse.requestTemperatures();          //get temperatures for first sensor
  *T1Cptr = sensor_inhouse.getTempCByIndex(0);   //get celsius value
  sensor_outhouse.requestTemperatures();         //get temperatures for second sensor
  *T2Cptr = sensor_outhouse.getTempCByIndex(0);  //get celsius value

  if (*T1Cptr < tempSet) {
    digitalWrite(relayPin, LOW);
    isRelay = 0;
  } else {
    digitalWrite(relayPin, HIGH);
    isRelay = 1;
  }

  isField = digitalRead(hallPin);  //read the hall sensor output

  initPage(1);  //draw the frame for page 1

  display.setCursor(25, 3);  // (x,y)

  display.println("Data Overview");  // Text or value to print

  display.setCursor(3, 14);  // (x,y)

  display.println("In:");  // Text or value to print

  printTemp1C(30, 14);  // (x,y)

  printUnit1C(68, 14);  // (x,y)

  display.setCursor(3, 22);  // (x,y)

  display.println("Out:");  // Text or value to print

  printTemp2C(30, 22);  // (x,y)

  printUnit2C(68, 22);  // (x,y)

  display.setCursor(3, 30);  // (x,y)

  display.println("Date:");  // Text or value to print

  display.setCursor(3, 38);  // (x,y)

  display.println("Time:");  // Text or value to print

  display.setCursor(3, 46);  // (x,y)

  display.println("Alarm1:");  // Text or value to print

  display.setCursor(46, 46);  // (x,y)

  display.println(tempSet);  // Text or value to print

  display.setCursor(3, 54);  // (x,y)

  display.println("Hall:");  // Text or value to print

  display.setCursor(32, 54);  // (x,y)

  display.println(isField);  // Text or value to print

  display.setCursor(80, 46);  // (x,y)

  display.println("Rly:");  // Text or value to print

  display.setCursor(104, 46);  // (x,y)

  if (isRelay == 1) {
    display.println("ON");
  } else {
    display.println("OFF");
  };

  display.display();  // Print everything we set previously

  return;
}

// this function reads and displays temperature sensors readings
void mySecondPage() {
  //print sensor data when displayMode equals 2
  sensor_inhouse.requestTemperatures();         //get temperatures for first sensor
  *T1Cptr = sensor_inhouse.getTempCByIndex(0);  //get celsius value
  *T1Fptr = sensor_inhouse.getTempFByIndex(0);  //get fahrenheit value
  *T1Kptr = *T1Cptr + 273.15;                   //convert celsius to kelvin

  sensor_outhouse.requestTemperatures();         //get temperatures for second sensor
  *T2Cptr = sensor_outhouse.getTempCByIndex(0);  //get celsius value
  *T2Fptr = sensor_outhouse.getTempFByIndex(0);  //get fahrenheit value
  *T2Kptr = *T2Cptr + 273.15;                    //convert celsius to kelvin

  initPage(2);  //draw the frame for page 2

  display.setCursor(17, 3);  // (x,y)

  display.println("Temperature Data");  // Text or value to print

  // first sensor display

  display.setCursor(3, 14);  // (x,y)

  display.println("In:");  // Text or value to print

  printTemp1C(30, 14);  // (x,y)

  printUnit1C(68, 14);  // (x,y)

  printTemp1F(30, 22);  // (x,y)

  printUnit1F(68, 22);  // (x,y)

  printTemp1K(30, 30);  // (x,y)

  printUnit1K(68, 30);  // (x,y)



  //second sensor display

  display.setCursor(3, 38);  // (x,y) 4th line

  display.println("Out:");  // Text or value to print

  printTemp2C(30, 38);  // (x,y)

  printUnit2C(68, 38);  // (x,y) 4th line

  printTemp2F(30, 46);  // (x,y)

  printUnit2F(68, 46);  // (x,y) 4th line

  printTemp2K(30, 54);  // (x,y)

  printUnit2K(68, 54);  // (x,y) 4th line

  display.display();  // Print everything we set previously

  return;
}

// this function reads and displays temperature alarms
void myThirdPage() {
  //display something when displayMode equals 3
  initPage(3);  //draw the frame for page 3

  display.setCursor(10, 3);  // (x,y)

  display.println("Temperature Alarms");  // Text or value to print

  display.display();  // Print everything we set previously

  return;
}

// this function reads and displays RTC data
void myFourthPage() {
  //display time and date when displayMode equals 4
  initPage(4);  //draw the frame for page 4

  display.setCursor(40, 3);  // (x,y)

  display.println("RTC Data");  // Text or value to print

  display.setCursor(3, 14);  // (x,y)

  display.println("Date:");  // Text or value to print

  display.setCursor(3, 22);  // (x,y)

  display.println("Time:");  // Text or value to print

  display.setCursor(3, 30);  // (x,y)

  display.println("Alarm1:");  // Text or value to print

  display.setCursor(3, 38);  // (x,y)

  display.println("Alarm2:");  // Text or value to print

  display.display();  // Print everything we set previously

  return;
}

// this function reads and displays GSM data
void myFifthPage() {
  //display something when displayMode equals 5
  initPage(5);  //draw the frame for page 5

  display.setCursor(40, 3);  // (x,y)

  display.println("GSM Data");  // Text or value to print

  display.display();  // Print everything we set previously

  return;
}


// this function reads and displays ambient light sensor readings
void mySixthPage() {

  //display something when displayMode equals 6
  initPage(6);  //draw the frame for page 6

  display.setCursor(10, 3);  // (x,y)

  display.println("Ambient Light Data");  // Text or value to print

  display.display();  // Print everything we set previously


  return;
}


// this function writes to sd
void mySeventhPage() {

  sensor_inhouse.requestTemperatures();         //get temperatures for first sensor
  *T1Cptr = sensor_inhouse.getTempCByIndex(0);  //get celsius value

  //display something when displayMode equals 7
  initPage(7);  //draw the frame for page 7

  display.setCursor(10, 3);  // (x,y)

  display.println("SDcard Logging");  // Text or value to print

  display.display();  // Print everything we set previously

  myFile = SD.open("log.txt", FILE_WRITE);
  if (myFile) {
   // myFile.print(sensor_inhouse.getTempCByIndex(0));
    myFile.print(",");
    myFile.close();  // close the file
  }
  // if the file didn't open, print an error:
  else {
    Serial.println("error opening test.txt");
  }
  delay(3000);

  return;
}

// this function draws the frame of the display, frame of the title and current page based on given input
void initPage(short a) {

  display.clearDisplay();  // Clear the display so we can refresh

  display.drawRect(0, 0, 128, 64, WHITE);  // Draw rectangle (x,y,width,height,color)

  display.drawLine(0, 12, 128, 12, WHITE);  // Draw rectangle (x,y,width,height,color)

  display.setCursor(89, 54);  // (x,y)

  display.println("pg.");  // Display page count

  display.setCursor(107, 54);  // (x,y)

  display.println(a);  // Display page count

  display.setCursor(113, 54);  // (x,y)

  display.println("/6");  // Display page count

  display.display();  // Print everything we set previously; if only frame is displayed, remove this line

  return;
}

// this function reads the desired display page, from serial monitor: 49==pg0. 50==pg1... (ascii codes of input symbols)
void checkDisplayMode() {

  displayMode = Serial.read();  //read input from serial interface

  if (displayMode != 49 && displayMode != 50 && displayMode != 51 && displayMode != 52 && displayMode != 53 && displayMode != 54 && displayMode != 55) {  //check for valid input value
    //Serial.println("Invalid Input!");                                                      //announce invalid input value; not used because se mai receptioneaza balarii pe interfata serial
    displayMode = oldDisplayMode;  //if value is valid, keep
  } else {
    oldDisplayMode = displayMode;  //if value is invalid, reset to previous value
    Serial.println(displayMode);   //print the input
  }
  return;
}

ISR(TIMER1_COMPA_vect) {  //timer1 interrupt 4Hz toggles reading input from serial monitor

  checkDisplayMode();  //when interrupt occurs assign the received value, to displayMode
  return;
}

// processing data of first temperature sensor

void printTemp1C(int x, int y) {
  display.setCursor(x, y);       //set cursor at given coordinates
  if (*T1Cptr == -127.0) {       // Detect if sensor 1 data line is disconnected
    display.println("Disc");     // Display sensor is disconnected
  } else if (*T1Cptr == 85.0) {  // Detect if sensor power is lost
    display.println("Pwr");      // Display sensor power is lost
  } else {
    display.println(*T1Cptr);  // Display Celsius reading for sensor 1
  };
  return;
}

void printTemp1F(int x, int y) {
  display.setCursor(x, y);       //set cursor at given coordinates
  if (*T1Cptr == -127.0) {       // Detect if sensor 1 data line is disconnected
    display.println("Disc");     // Display sensor is disconnected
  } else if (*T1Cptr == 85.0) {  // Detect if sensor power is lost
    display.println("Pwr");      // Display sensor power is lost
  } else {
    display.println(*T1Fptr);  // Display Fahrenheit reading for sensor 1
  };
  return;
}

void printTemp1K(int x, int y) {
  display.setCursor(x, y);       //set cursor at given coordinates
  if (*T1Cptr == -127.0) {       // Detect if sensor 1 data line is disconnected
    display.println("Disc");     // Display sensor is disconnected
  } else if (*T1Cptr == 85.0) {  // Detect if sensor power is lost
    display.println("Pwr");      // Display sensor power is lost
  } else {
    display.println(*T1Kptr);  // Display Kelvin reading for sensor 1
  };
  return;
}


// processing data of second temperature sensor

void printTemp2C(int x, int y) {
  display.setCursor(x, y);       //set cursor at given coordinates
  if (*T2Cptr == -127.0) {       // Detect if sensor 2 data line is disconnected
    display.println("Disc");     // Display sensor is disconnected
  } else if (*T2Cptr == 85.0) {  // Detect if sensor power is lost
    display.println("Pwr");      // Display sensor power is lost
  } else {
    display.println(*T2Cptr);  // Display Celsius reading for sensor 2
  };
  return;
}

void printTemp2F(int x, int y) {
  display.setCursor(x, y);       //set cursor at given coordinates
  if (*T2Cptr == -127.0) {       // Detect if sensor 2 data line is disconnected
    display.println("Disc");     // Display sensor is disconnected
  } else if (*T2Cptr == 85.0) {  // Detect if sensor power is lost
    display.println("Pwr");      // Display sensor power is lost
  } else {
    display.println(*T2Fptr);  // Display Fahrenheit reading for sensor 2
  };
  return;
}

void printTemp2K(int x, int y) {
  display.setCursor(x, y);       //set cursor at given coordinates
  if (*T2Cptr == -127.0) {       // Detect if sensor 2 data line is disconnected
    display.println("Disc");     // Display sensor is disconnected
  } else if (*T2Cptr == 85.0) {  // Detect if sensor power is lost
    display.println("Pwr");      // Display sensor power is lost
  } else {
    display.println(*T2Kptr);  // Display Kelvin reading for sensor 2
  };
  return;
}


// display measurement units for first sensor

void printUnit1C(int x, int y) {
  display.setCursor(x, y);                     //set cursor at given coordinates
  if (*T1Cptr == -127.0 || *T1Cptr == 85.0) {  //check if error
    display.println();                         // Print nothing if error
  } else {
    display.println("C");  //Print unit if all good
  };
  return;
}

void printUnit1F(int x, int y) {
  display.setCursor(x, y);                     //set cursor at given coordinates
  if (*T1Cptr == -127.0 || *T1Cptr == 85.0) {  //check if error
    display.println();                         // Print nothing if error
  } else {
    display.println("F");  //Print unit if all good
  };
  return;
}

void printUnit1K(int x, int y) {
  display.setCursor(x, y);                     // Set cursor at given coordinates
  if (*T1Cptr == -127.0 || *T1Cptr == 85.0) {  // Check if error
    display.println();                         // Print nothing if error
  } else {
    display.println("K");  //Print unit if all good
  };
  return;
}

// display measurement units for second sensor

void printUnit2C(int x, int y) {
  display.setCursor(x, y);                     // Set cursor at given coordinates
  if (*T2Cptr == -127.0 || *T2Cptr == 85.0) {  // Check if error
    display.println();                         // Print nothing if error
  } else {
    display.println("C");  //Print unit if all good
  };
  return;
}

void printUnit2F(int x, int y) {
  display.setCursor(x, y);                     //set cursor at given coordinates
  if (*T2Cptr == -127.0 || *T2Cptr == 85.0) {  //check if error
    display.println();                         // Print nothing if error
  } else {
    display.println("F");  //Print unit if all good
  };
  return;
}

void printUnit2K(int x, int y) {
  display.setCursor(x, y);                     //set cursor at given coordinates
  if (*T2Cptr == -127.0 || *T2Cptr == 85.0) {  //check if error
    display.println();                         // Print nothing if error
  } else {
    display.println("K");  //Print unit if all good
  };
  return;
}