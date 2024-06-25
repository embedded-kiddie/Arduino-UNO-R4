# Playing music while blinking an LED

It makes an LED blinking with playing music [`TRUTH by T-SQUARE`](https://www.youtube.com/watch?v=liuNBOXGJxg) (2016) in background.

https://github.com/embedded-kiddie/Arduino-UNO-R4/assets/159898757/0616f5a3-ce63-4d86-8820-b6b0181a1354

## Dependency

- CallbackTimerR4
  - https://github.com/embedded-kiddie/CallbackTimerR4
- BackgroundMusicR4
  - https://github.com/embedded-kiddie/BackgroundMusicR4

## How to install
1. Connect one end of the piezoelectric sounder to pin 9 of UNO R4 and the other end to the GND pin.
  <img width="500" alt="background_music" src="https://github.com/embedded-kiddie/Arduino-UNO-R4/assets/159898757/9fb1aa93-5aff-4d30-b73b-38deeb4cec3a">

2. Download the latest library `.zip` files.
  - [CallbackTimerR4](https://github.com/embedded-kiddie/CallbackTimerR4/archive/refs/tags/v1.0.1.zip "Release Fixed undefined symbol &#39;LED_TX&#39; and &#39;LED_RX&#39; errors on UNO R4 WiFi. · embedded-kiddie/CallbackTimerR4")
  - [BackgroundMusicR4](https://github.com/embedded-kiddie/BackgroundMusicR4/archive/refs/tags/v1.0.0.zip "Release First release. · embedded-kiddie/BackgroundMusicR4")

3. Install downloaded `.zip` files from "**Sketch** --> **Include Library** --> **Add .ZIP Library...**" on Arduino IDE menu.

4. The verify and upload (compile) the `background_music.ino` to enjoy it!

## Blog post
  - [Playing music while blinking an LED - Arduino UNO R4 timers, interrupts and clocks](https://bit.ly/3VQQAdj)
