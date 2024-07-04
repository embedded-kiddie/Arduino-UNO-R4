#include <ctype.h>

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial);
#ifdef ARDUINO_UNOR4_WIFI
  delay(1000); // It requires at least 600 ms to complete Serial initialization.
#endif
}

void loop() {
  // put your main code here, to run repeatedly:
#if 0
  if (Serial.available()) {
    String s = Serial.readString();
    Serial.print(s);
  }
#else
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
        Serial.print((int)c);
      }
      Serial.println();
    }
    Serial.println("n+1 --> " + String((int)s[len+1]));
  }
#endif
}
