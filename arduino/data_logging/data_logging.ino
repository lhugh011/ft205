#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <RTClib.h>

// HC-SE04 distance sensor pins
const int pingPin = 7; // trigger pin of ultrasonic sensor
const int echoPin = 6; // echo pin of ultrasonic sensor

#define LOG_INTERVAL 1000 // mills between entries
#define SYNC_INTERVAL 1000 // mills between calls to flush() to write data to SD card
uint32_t syncTime = 0; // time of last sync()
#define ECHO_TO_SERIAL 1 // echo to serial port
#define WAIT_TO_START 0 // wait for serial input in setup()

// digital pins connected to debugging LEDs
#define greenLEDpin 2
#define redLEDpin 3

RTC_PCF8523 RTC; // define the real time clock object

// for data logging shield use digital pin 10 for the SD cs line
const int chipSelect = 10;

// logging file
File logfile;

// file creation timestamp block
// callback for file timestamps
void dateTime(uint16_t* date, uint16_t* time) {
  DateTime now = RTC.now();

  // return date using FAT_DATE macro to format fields
  *date = FAT_DATE(now.year(), now.month(), now.day());

  // return time using FAT_TIME macro to format fields
  *time = FAT_TIME(now.hour(), now.minute(), now.second());
}
// end file creation timestamp block

void error(char *str)
{
  Serial.print("error: ");
  Serial.println(str);

  // red LED indicates error
  digitalWrite(redLEDpin, HIGH);

  while(1);
}


void setup(void)
{
   Serial.begin(9600); // starting serial terminal
   Serial.println();

  // enable debugging LEDs
  pinMode(redLEDpin, OUTPUT);
  pinMode(greenLEDpin, OUTPUT);

// if WAIT_TO_START
#if WAIT_TO_START
  Serial.println("Type any character to start");
  while(!Serial.available());
#endif // WAIT_TO_START

  // initialize SD card
  Serial.print("Initializing SD card...");
  pinMode(10, OUTPUT); // chip select pin 10

  if (!SD.begin(chipSelect)) {
    error("Card failed, or not present");
  }
  Serial.println("card initialized.");

  // connect to RTC
  Wire.begin();
  if (!RTC.begin()) {
    logfile.println("RTC failed");
  #if ECHO_TO_SERIAL
    Serial.println("RTC failed");
  #endif // ECHO_TO_SERIAL
  }
  // callback for file timestamps
  SdFile::dateTimeCallback(dateTime);

  // create a new file
  char filename[] = "LOGGER00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = i/10 + '0';
    filename[7] = i%10 + '0';
    if (!SD.exists(filename)) {
      // open new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE);
      break; // exit loop
    }
  }

  if (!logfile) {
    error("could not create file");
  }

  Serial.print("Logging to: ");
  Serial.println(filename);

  logfile.println("unix_time, time_stamp, dist_in, dist_cm");
#if ECHO_TO_SERIAL
  Serial.println("unix_time, time_stamp, dist_in, dist_cm");
#endif // ECHO_TO_SERIAL


} // end void setup


void loop(void)
{
  DateTime now;

  // delay for the amount of time between readings
  delay((LOG_INTERVAL - 1) - (millis() % LOG_INTERVAL));

  // green LED indicates OK
  digitalWrite(greenLEDpin, HIGH);

  // log mills since starting
/*
  uint32_t m = millis();
  logfile.print(m); // mills since start
  logfile.print(", ");
#if ECHO_TO_SERIAL
  Serial.print(m); // mills since start
  Serial.print(", ");
#endif
*/

  // fetch time
  now = RTC.now();
  // log time
  logfile.print(now.unixtime()); // seconds since 1/1/1970
  logfile.print(", ");
  logfile.print(now.year(), DEC);
  logfile.print("/");
  logfile.print(now.month(), DEC);
  logfile.print("/");
  logfile.print(now.day(), DEC);
  logfile.print(" ");
  logfile.print(now.hour(), DEC);
  logfile.print(":");
  logfile.print(now.minute(), DEC);
  logfile.print(":");
  logfile.print(now.second(), DEC);
  logfile.print(", ");
  //logfile.print("\r");
#if ECHO_TO_SERIAL
  Serial.print(now.unixtime()); // seconds since 1/1/1970
  Serial.print(", ");
  Serial.print(now.year(), DEC);
  Serial.print("/");
  Serial.print(now.month(), DEC);
  Serial.print("/");
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(":");
  Serial.print(now.minute(), DEC);
  Serial.print(":");
  Serial.print(now.second(), DEC);
  Serial.print(", ");
  //Serial.print("\r\n");
#endif //ECHO_TO_SERIAL


  // distance sensor code block
  long duration, inches, cm;
  pinMode(pingPin, OUTPUT);
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(pingPin, LOW);
  pinMode(echoPin, INPUT);
  duration = pulseIn(echoPin, HIGH);
  inches = microsecondsToInches(duration);
  cm = microsecondsToCentimeters(duration);
  //log data
  logfile.print(inches);
  logfile.print(", ");
  logfile.print(cm);
  logfile.print("\r");
#if ECHO_TO_SERIAL
  Serial.print(inches);
  Serial.print("in, ");
  Serial.print(cm);
  Serial.print("cm");
  Serial.println();
  delay(100);
#endif //ECHO_TO_SERIAL

  // write to SD card
  if ((millis() - syncTime) < SYNC_INTERVAL) return;
  syncTime = millis();

  // blink debug LEDs to syncing data to SD and updating FAT
  digitalWrite(redLEDpin, HIGH);
  logfile.flush();
  digitalWrite(redLEDpin, LOW);


} // end void loop

long microsecondsToInches(long microseconds) {
   return microseconds / 74 / 2;
}

long microsecondsToCentimeters(long microseconds) {
   return microseconds / 29 / 2;
}
