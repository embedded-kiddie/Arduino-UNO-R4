/*
  Minima
    ** Serial Speed Test **
    !Serial = 1
    Twas brillig and the slithy toves did gyre and gimbel in the wabe.
    Total Time : 207

  WiFi
    ** Serial Speed Test **
    !Serial = 0
    Twas brillig and the slithy toves did gyre and gimbel in the wabe.
    Total Time : 69792
*/
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  bool t = !Serial;
  while (!Serial);

#ifdef  ARDUINO_UNOR4_WIFI
  // UNO R4 WiFi needs to wait for a while to complete Serial initialization.
  delay(1000); // It requires at least 600 ms.
#endif

  Serial.println("\n** Serial Speed Test **");
  Serial.println("!Serial = " + String(t));

  uint32_t start = micros();
  Serial.print("Twas brillig and the slithy toves did gyre and gimbel in the wabe.\n");
  uint32_t finish = micros();

  Serial.println("Total Time : " + String(finish - start));
}

void loop() {
  // put your main code here, to run repeatedly:

}
