//
// FILE: TwoDS18B20_OneOLED.ino
// AUTHOR: Vlad Maris
// VERSION: 0.2.00
// PURPOSE: two pins for two sensors demo, with OLED display on I2C
// DATE: 2022-10-23
//
#include <stdio.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_GFX.h>      // Include core graphics library for the display
#include <Adafruit_SSD1306.h>  // Include Adafruit_SSD1306 library to drive the display


#define ONE_WIRE_BUS_1 11
#define ONE_WIRE_BUS_2 10

OneWire oneWire_t1(ONE_WIRE_BUS_1);
OneWire oneWire_t2(ONE_WIRE_BUS_2);

DallasTemperature sensor_inhouse(&oneWire_t1);
DallasTemperature sensor_outhouse(&oneWire_t2);

Adafruit_SSD1306 display(128, 64);  // Create display

float T1C, T2C, T1F, T2F, T1K, T2K;

int displayMode;
int oldDisplayMode;
int val;

void setup(void) {

  //  delay(100);  // This delay is needed to let the display to initialize

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

  Serial.begin(9600);

  Serial.println("Two Sensors, One Display");


  display.setTextColor(WHITE);  // Set color of the text

  display.setRotation(0);  // Set orientation. Goes from 0, 1, 2 or 3

  display.setTextWrap(false);  // By default, long lines of text are set to automatically ΓÇ£wrapΓÇ¥ back to the leftmost column.

  display.setTextSize(0);  // Set text size. We are using a custom font so you should always use the text size of 0

  display.dim(1);  //Set brightness (0 is maximun and 1 is a little dim)

  displayMode = 48;

  oldDisplayMode = 48;
}

void loop(void) {

  if (displayMode == 48) {         //if 0 is read
    myFirstThread();               // run first thread
  } else if (displayMode == 49) {  //if 1
    mySecondThread();              // run second thread
  } else if (displayMode == 50) {  // if 2
    myThirdThread();               //run third thread
  } else if (displayMode == 51) {  //if 3
    myFourthThread();              // run fourth thread
  } else if (displayMode == 52) {  //if 4
    myFifthThread();               // run fifth thread
  }
}


void myFirstThread() {

  sensor_inhouse.requestTemperatures();      //get temperatures for first sensor
  T1C = sensor_inhouse.getTempCByIndex(0);   //get celsius value
  sensor_outhouse.requestTemperatures();     //get temperatures for second sensor
  T2C = sensor_outhouse.getTempCByIndex(0);  //get celsius value


  initPage();

  display.println("pg.0/4");  // Display page count

  display.setCursor(30, 3);  // (x,y)

  display.println("Data Overview");  // Text or value to print

  display.setCursor(3, 14);  // (x,y)

  display.println("In:");  // Text or value to print

  display.setCursor(30, 14);  // (x,y)

  if (T1C == -127.0) {
    display.println("Disc");  // Text or value to print
  } else if (T1C == 85.0) {
    display.println("Pwr");
  } else {
    display.println(T1C);
  };

  display.setCursor(68, 14);  // (x,y)

  if (T1C == -127.0 || T1C == 85.0) {
    display.println("");  // Text or value to print
  } else {
    display.println("C");
  };  // Text or value to print}

  display.setCursor(3, 22);  // (x,y)

  display.println("Out:");  // Text or value to print

  display.setCursor(30, 22);  // (x,y)
  if (T2C == -127.0) {
    display.println("Disc");  // Text or value to print
  } else if (T2C == 85.0) {
    display.println("Pwr");
  } else {
    display.println(T2C);
  };

  display.setCursor(68, 22);  // (x,y)

  if (T2C == -127.0 || T2C == 85.0) {
      display.println("");  // Text or value to print
    }
  else {
    display.println("C");
  };  // Text or value to print}

  display.setCursor(3, 30);  // (x,y)

  display.println("Date:");  // Text or value to print

  display.setCursor(3, 38);  // (x,y)

  display.println("Time:");  // Text or value to print

  display.setCursor(3, 46);  // (x,y)

  display.println("Alarm1:");  // Text or value to print

  display.display();  // Print everything we set previously
  return;
}

void mySecondThread() {
  //print sensor data when displayMode equals 1

  sensor_inhouse.requestTemperatures();     //get temperatures for first sensor
  T1C = sensor_inhouse.getTempCByIndex(0);  //get celsius value
  T1F = sensor_inhouse.getTempFByIndex(0);  //get fahrenheit value
  T1K = T1C + 273.15;                       //convert celsius to kelvin

  sensor_outhouse.requestTemperatures();     //get temperatures for second sensor
  T2C = sensor_outhouse.getTempCByIndex(0);  //get celsius value
  T2F = sensor_outhouse.getTempFByIndex(0);  //get fahrenheit value
  T2K = T2C + 273.15;                        //convert celsius to kelvin

  initPage();

  display.println("pg.1/4");  // Display page count

  display.setCursor(17, 3);  // (x,y)

  display.println("Temperature Data");  // Text or value to print

  // first sensor display

  display.setCursor(3, 14);  // (x,y)

  display.println("In:");  // Text or value to print

  display.setCursor(30, 14);  // (x,y)

  if (T1C == -127.0) {
    display.println("Disc");  // Text or value to print
  } else if (T1C == 85.0) {
    display.println("Pwr");
  } else {
    display.println(T1C);
  };

  display.setCursor(68, 14);  // (x,y)

  if (T1C == -127.0 || T1C == 85.0) {
    display.println("");  // Text or value to print
  } else {
    display.println("C");
  };  // Text or value to print}

  display.setCursor(30, 22);  // (x,y)

  if (T1C == -127.0) {
    display.println("Disc");  // Text or value to print
  } else if (T1C == 85.0) {
    display.println("Pwr");
  } else {
    display.println(T1F);
  };

  display.setCursor(68, 22);  // (x,y)

  if (T1C == -127.0 || T1C == 85.0) {
    display.println("");  // Text or value to print
  } else {
    display.println("F");
  };  // Text or value to print}

  display.setCursor(30, 30);  // (x,y)

  if (T1C == -127.0) {
    display.println("Disc");  // Text or value to print
  } else if (T1C == 85.0) {
    display.println("Pwr");
  } else {
    display.println(T1K);
  };

  display.setCursor(68, 30);  // (x,y)

  if (T1C == -127.0 || T1C == 85.0) {
    display.println("");  // Text or value to print
  } else {
    display.println("K");
  };  // Text or value to print}

  //second sensor display

  display.setCursor(3, 38);  // (x,y) 4th line

  display.println("Out:");  // Text or value to print

  display.setCursor(30, 38);  // (x,y) 4th line

  if (T2C == -127.0) {
    display.println("Disc");  // Text or value to print
  } else if (T2C == 85.0) {
    display.println("Pwr");
  } else {
    display.println(T2C);
  };

  display.setCursor(68, 38);  // (x,y) 4th line

  if (T2C == -127.0 || T2C == 85.0) {
    display.println("");  // Text or value to print
  } else {
    display.println("C");
  };  // Text or value to print}

  display.setCursor(30, 46);  // (x,y) 4th line

  if (T2C == -127.0) {
    display.println("Disc");  // Text or value to print
  } else if (T2C == 85.0) {
    display.println("Pwr");
  } else {
    display.println(T2F);
  };

  display.setCursor(68, 46);  // (x,y) 4th line

  if (T2C == -127.0 || T2C == 85.0) {
    display.println("");  // Text or value to print
  } else {
    display.println("F");
  };  // Text or value to print}

  display.setCursor(30, 54);  // (x,y) 4th line

  if (T2C == -127.0) {
    display.println("Disc");  // Text or value to print
  } else if (T2C == 85.0) {
    display.println("Pwr");
  } else {
    display.println(T2K);
  };

  display.setCursor(68, 54);  // (x,y) 4th line

  if (T2C == -127.0 || T2C == 85.0) {
    display.println("");  // Text or value to print
  } else {
    display.println("K");
  };  // Text or value to print}

  display.display();  // Print everything we set previously

  return;
}

void myThirdThread() {
  //display something when displayMode equals 3
  initPage();

  display.println("pg.2/4");  // Display page count

  display.setCursor(13, 3);  // (x,y)

  display.println("Temperature Alarms");  // Text or value to print

  display.display();  // Print everything we set previously

  return;
}

void myFourthThread() {
  //display time and date when displayMode equals 2
  initPage();

  display.println("pg.3/4");  // Display page count

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

void myFifthThread() {
  //display something when displayMode equals 3
  initPage();

  display.println("pg.4/4");  // Display page count

  display.setCursor(40, 3);  // (x,y)

  display.println("GSM Data");  // Text or value to print

  display.display();  // Print everything we set previously

  return;
}

void initPage() {
  display.clearDisplay();  // Clear the display so we can refresh

  display.drawRect(0, 0, 128, 64, WHITE);  // Draw rectangle (x,y,width,height,color)

  display.drawLine(0, 12, 128, 12, WHITE);  // Draw rectangle (x,y,width,height,color)

  display.setCursor(89, 54);  // (x,y)
}

void checkDisplayMode() {

  displayMode = Serial.read();  //read input from serial interface

  if (displayMode != 48 && displayMode != 49 && displayMode != 50 && displayMode != 51 && displayMode != 52) {  //check for input value
    //Serial.println("Invalid Input!");                                                      //announce invalid input value
    displayMode = oldDisplayMode;  //if value is valid, keep
  } else {
    oldDisplayMode = displayMode;  //if value is invalid, reset to previous value
    Serial.println(displayMode);   //print the input
  }
}

ISR(TIMER1_COMPA_vect) {  //timer1 interrupt 4Hz toggles reading input from serial monitor

  checkDisplayMode();
}