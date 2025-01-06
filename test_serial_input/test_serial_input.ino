#include <ctype.h>
#include <stdio.h>

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial);
#ifdef ARDUINO_UNOR4_WIFI
  delay(1000); // It requires at least 600 ms to complete Serial initialization.
#endif
}

void loop() {
#if 0
  // Serial.readString
  if (Serial.available()) {
    String s = Serial.readString();
    Serial.print(s);
  }
#else
  // Serial.readBytes
  if (Serial.available()) {
    char s[256];
    memset(s, -1, sizeof(s)); // fill 0xff

    int len = Serial.readBytes(s, sizeof(s)); // it is not terminated by `\0`
    Serial.println("len = " + String(len));
    
    for (int i = 0; i < len; i++) {
      char c = s[i];
      if (!iscntrl(c) && isprint(c)) {
        Serial.print(c);
      }
      else {
        Serial.print("0x" + String(c, HEX));
      }
      Serial.println();
    }
    Serial.println((int)s[len+1]);
    sscanf(s, "%d", &len);
  }
#endif
}
