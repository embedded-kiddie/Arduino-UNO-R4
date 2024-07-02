/*
  Minima
    ** Serial Speed Test **
    !Serial = 1
    Serial.available() = 0
    Serial.availableForWrite() = 256
    Twas brillig and the slithy toves did gyre and gimble in the wabe.
    Total Time : 224

  WiFi
    ** Serial Speed Test **
    !Serial = 0
    Serial.available() = 0
    Serial.availableForWrite() = 0
    Twas brillig and the slithy toves did gyre and gimble in the wabe.
    Total Time : 69793
*/
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  bool t = !Serial;
  while (!Serial);

#ifdef  ARDUINO_UNOR4_WIFI
  delay(1000); // It requires at least 600 ms to complete Serial initialization.
#endif

  Serial.println("\n** Serial Speed Test **");
  Serial.println("!Serial = " + String(t));
  Serial.println("Serial.available() = " + String(Serial.available()));
  Serial.println("Serial.availableForWrite() = " + String(Serial.availableForWrite()));

  uint32_t start = micros();
  Serial.print("Twas brillig and the slithy toves did gyre and gimble in the wabe.\n");
  uint32_t finish = micros();

  Serial.println("Total Time : " + String(finish - start));
}

void loop() {}
