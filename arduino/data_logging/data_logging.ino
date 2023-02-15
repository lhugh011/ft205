#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <RTClib.h>
#include <SoftwareSerial.h>

// pin6 UNO RX --> connect to FT205 TX (Yellow)
// pin7 UNO TX --> connect to FT205 RX (Blue)
SoftwareSerial mySerial(6, 7);

#define LOG_INTERVAL 1000 // mills between entries
#define SYNC_INTERVAL 1000 // mills between calls to flush() to write data to SD card
uint32_t syncTime = 0; // time of last sync()
#define ECHO_TO_SERIAL 1 // echo to serial port
#define WAIT_TO_START 0 // wait for serial input in setup()
#define greenLEDpin 2 // IO pin 2 green debug LED
#define redLEDpin 3 // IO pin 3 red debug LED
RTC_PCF8523 RTC; // define the real time clock object
const int chipSelect = 10; // for data logging shield use digital pin 10 for the SD cs line

File logfile; // logging file object


// MUST REMAIN TOP VOID FUNCTION
//---------------file creation timestamp----------------
// callback for file timestamps
void dateTime(uint16_t* date, uint16_t* time) {
  DateTime now = RTC.now();

  // return date using FAT_DATE macro to format fields
  *date = FAT_DATE(now.year(), now.month(), now.day());

  // return time using FAT_TIME macro to format fields
  *time = FAT_TIME(now.hour(), now.minute(), now.second());
}
//---------------end file creation timestamp------------


void error(char *str)
{
  Serial.print("error: ");
  Serial.println(str);

  digitalWrite(redLEDpin, HIGH); // red LED indicates error

  while(1);
}


void setup(void)
{
  Serial.begin(9600);
  Serial.println();

  mySerial.begin(9600);
  Serial.print("Wind sensor initializing...\r\n");
  delay(3000);
  mySerial.write("$01CIU*//\r\n"); // SET ft205ev to UART mode
  delay(2000);

  // enable debugging LEDs
  pinMode(redLEDpin, OUTPUT);
  pinMode(greenLEDpin, OUTPUT);

// if WAIT_TO_START
#if WAIT_TO_START
  Serial.println("Type any character to start");
  while(!Serial.available());
#endif

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
  #endif
  }

  SdFile::dateTimeCallback(dateTime); // callback for file timestamps

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
// create headers
  logfile.println("unix_time, time_stamp, talker_id, speed (m/s), angle (deg), status");
#if ECHO_TO_SERIAL
  Serial.println("unix_time, time_stamp, talker_id, speed (m/s), angle (deg), status");
#endif

} // end void setup


void loop(void)
{
  DateTime now;

//---------------------DO NOT DELETE------------------------
  // delay amount of time between readings
  //delay((LOG_INTERVAL - 1) - (millis() % LOG_INTERVAL));  
//----------------------------------------------------------

  digitalWrite(greenLEDpin, HIGH); // green LED indicates OK
  now = RTC.now();  // fetch time

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
#endif

  // log data
  mySerial.write("$//WV?*//\r\n"); // QUERY wind speed and direction msg
  logfile.print(mySerial.readString());
#if ECHO_TO_SERIAL
  Serial.println();
#endif

//--------------------DO NOT DELETE----------------------
  // write to SD card
  //if ((millis() - syncTime) < SYNC_INTERVAL) return;
  //syncTime = millis();
//-------------------------------------------------------

  // blink debug LEDs to syncing data to SD and updating FAT
  digitalWrite(redLEDpin, HIGH);
  logfile.flush();
  digitalWrite(redLEDpin, LOW);

} // end void loop
