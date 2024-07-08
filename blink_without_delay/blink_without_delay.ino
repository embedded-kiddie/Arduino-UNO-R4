void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
}

#if 1

// https://docs.arduino.cc/built-in-examples/digital/BlinkWithoutDelay/

int ledState = LOW;  // ledState used to set the LED

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;  // will store last time LED was updated

// constants won't change:
const long interval = 1000;  // interval at which to blink (milliseconds)

void loop() {
  // here is where you'd put code that needs to be running all the time.

  // check to see if it's time to blink the LED; that is, if the difference
  // between the current time and last time you blinked the LED is bigger than
  // the interval at which you want to blink the LED.
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
//  previousMillis = currentMillis;
    previousMillis += interval;

    // set the LED with the ledState of the variable:
    digitalWrite(LED_BUILTIN, ledState = !ledState);
  }
}

#elif 0

// https://forum.arduino.cc/t/is-there-a-non-blocking-delay-thats-as-easy-to-use-as-the-built-in-delay-function/1195423/66

boolean delay_without_delaying(unsigned long &previousMillis, unsigned long period) {
  // return false if we're still "delaying", true if time ms has passed.
  // this should look a lot like "blink without delay"
  unsigned long currentmillis = millis();
  if (currentmillis - previousMillis >= period) {
    previousMillis = currentmillis;
    return true;
  }
  return false;
}

void loop() {
  static unsigned long ledtime = 0;
  static unsigned long atime, btime, ctime, nltime;
  static int ledState = false;
  if (delay_without_delaying(ledtime, 1000)) {
    digitalWrite(LED_BUILTIN, ledState = !ledState);
  }
  if (delay_without_delaying(atime, 500)) {
    Serial.print("A");
  }
  if (delay_without_delaying(btime, 1000)) {
    Serial.print("B");
  }
  if (delay_without_delaying(ctime, 2000)) {
    Serial.print("C");
  }
  if (delay_without_delaying(nltime, 5000)) {
    Serial.println("D");
  }
}

#elif 0

// https://forum.arduino.cc/t/is-there-a-non-blocking-delay-thats-as-easy-to-use-as-the-built-in-delay-function/1195423/68

#define DO_EVERY(interval) for (static unsigned long previousMillis = millis(); millis() - previousMillis >= interval; previousMillis = millis())

void loop() {

  DO_EVERY(500){
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    Serial.print("A");
  }
  DO_EVERY(1000){
    Serial.print("B");
  }
  DO_EVERY(2000){
    Serial.print("C");
  }
  DO_EVERY(5000){
    Serial.println("D");
  }
}

#else

// https://forum.arduino.cc/t/is-there-a-non-blocking-delay-thats-as-easy-to-use-as-the-built-in-delay-function/1195423/144

#define onTimeout(previousMillis, period) \
  static unsigned long previousMillis = millis(); \
  for (unsigned long currentmillis = millis(); (currentmillis - previousMillis) >= (unsigned long)period; previousMillis = currentmillis)

void loop() {
  onTimeout(timerA, 500) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    Serial.print("A");
  }
  onTimeout(timerB, 1000) {
    Serial.print("B");
  }
  onTimeout(timerC, 2000) {
    Serial.print("C");
  }
  onTimeout(timerD, 5000) {
    Serial.println("D");
  }
}

#endif