# Serial output speed test for MINIMA and WIFI 

## Issue 1: Speed

I'm inspired by [The R4 Serial Problem - UNO R4 Minima - Arduino Forum](https://forum.arduino.cc/t/the-r4-serial-problem/1255584/).

The original code:

```c++
uint32_t start;
uint32_t finish;

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);

  while(!Serial);

  Serial.println("\n\n** Speed Test **\n\n");

  start = micros();
  Serial1.print("Twas brillig and the slithy toves did gyre and gimbel in the wabe. ");
  finish = micros();

  Serial.print("Total Time : ");
  Serial.println(finish - start);
}

void loop() {}
```

I modified `Serial1` to `Serial` to test MINIMA and WIFI serial output speed, and I got:

| Minima | WiFi  |
| ------:| -----:|
| 207    | 69793 |

I think the reason why the WiFi communication speed is so slow is because all serial input/output is passed through the ESP32 via a level translator IC.

![UNO R4 WiFi Level translator and ESP32-S3-MINI-1-N8](https://github.com/embedded-kiddie/Arduino-UNO-R4/assets/159898757/c963348e-575d-433a-8b19-c84ee077adee "UNO R4 WiFi Level translator and ESP32-S3-MINI-1-N8")

## Issue 2: Initialization

WiFi's operator `bool` of `Serial` returns `true` immediately. So the following code does not make sense.

```c++
  Serial.begin(9600);
  while (!Serial);
```

or

```c++
  Serial.begin(9600);
  while (!Serial && millis() < 1000);
```

Instead of the code above, use:

```c++
  Serial.begin(9600);

#ifdef  ARDUINO_UNOR4_WIFI
  delay(1000); // It requires at least 600 ms to complete Serial initialization.
#endif
```

## Issue 3: Availability

- `Serial.available()`  
  Both Minima and WiFi always return `0`.

- `Serial.availableForWrite()`  
  Minima returns `256` but WiFi returns `0`

cf. Check if `Serial.flush()` is blocking or non-blocking.

[Serial.availableForWrite()]: https://www.arduino.cc/reference/en/language/functions/communication/serial/availableforwrite/ "Serial.availableForWrite() - Arduino Reference"

[Serial.flush()]: https://www.arduino.cc/reference/en/language/functions/communication/serial/flush/ "Serial.flush() - Arduino Reference"

[Serial Communications (SoftwareSerial Class)]: https://www.renesas.com/us/en/products/gadget-renesas/reference/gr-kurumi/library-softwareserial "Serial Communications (SoftwareSerial Class) - Renesas"

## Related issues
- [UNOWIFOR4 - Sketch that uses HID - Some Serial.print(...) don't output. #135](https://github.com/arduino/ArduinoCore-renesas/issues/135 "UNOWIFOR4 - Sketch that uses HID - Some Serial.print(...) don&#39;t output. · Issue #135 · arduino/ArduinoCore-renesas")
- [USB Serial - differences between WIFI and MINIMA](https://forum.arduino.cc/t/usb-serial-differences-between-wifi-and-minima/1154648 "USB Serial - differences between WIFI and MINIMA - UNO R4 WiFi - Arduino Forum")
- [While (!Serial) {}; Not Sufficient On Upload or USB Plugin](https://forum.arduino.cc/t/while-serial-not-sufficient-on-upload-or-usb-plugin/1168429 "While (!Serial) {}; Not Sufficient On Upload or USB Plugin - UNO R4 WiFi - Arduino Forum")
