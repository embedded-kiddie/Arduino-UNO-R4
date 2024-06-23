# Program to test MINIMA and WIFI serial output speed

Inspired by [The R4 Serial Problem - UNO R4 Minima - Arduino Forum](https://forum.arduino.cc/t/the-r4-serial-problem/1255584/).

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
| 207    | 69792 |

I think the reason why the WiFi communication speed is so slow is because all serial input/output is passed through the ESP32 via a level conversion IC.

![UNO R4 WiFi Level translator and ESP32-S3-MINI-1-N8](https://github.com/embedded-kiddie/Arduino-UNO-R4/assets/159898757/c963348e-575d-433a-8b19-c84ee077adee "UNO R4 WiFi Level translator and ESP32-S3-MINI-1-N8"){: width="1126" height="1544" }

