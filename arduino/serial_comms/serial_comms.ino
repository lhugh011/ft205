#include <SoftwareSerial.h>

// pin6 UNO RX --> connect to FT205 TX (Yellow)
// pin7 UNO TX --> connect to FT205 RX (Blue)
SoftwareSerial mySerial(6, 7);

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  Serial.print("Wind sensor initializing...\r\n");
  delay(3000);
  // SET ft205ev to UART mode
  mySerial.write("$01CIU*//\r\n");
  delay(2000);
  // QUERY if comms protocol is set
  mySerial.write("$01CI?*//\r\n");
  delay(1000);

}

void loop() {
  // QUERY wind speed and direction msg
  mySerial.write("$//WV?*//\r\n");
  while(mySerial.available())
  {
    Serial.write(mySerial.read());
  }
  Serial.println();
  delay(1000);
}
