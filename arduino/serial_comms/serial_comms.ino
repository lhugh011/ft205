#include <SoftwareSerial.h>

// pin6 UNO RX --> connect to FT205 TX (Yellow)
// pin7 UNO TX --> connect to FT205 RX (Blue)
SoftwareSerial mySerial(6, 7);

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  Serial.print("Wind sensor initialize");
  delay(3000);
  mySerial.write("$01CIU*//\r\n");
  delay(2000);
  mySerial.write("$01CI?*//\r\n");
  delay(1000);

}

void loop() {
  mySerial.write("$//WV?*//\r\n");
  while(mySerial.available())
  {
    Serial.write(mySerial.read());
  }
  Serial.println();
  delay(1000);

}
