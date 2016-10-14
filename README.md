# StrangerBoard

An Arduino sketch created for [Veracode][0] Hackathon X; it simulates the Stranger Things show Christmas-light message display.
You can connect to it over USBserial using 9600-N-1.

There is a [Python library and application][1] for talking to this board using PySerial.

[0]: https://www.veracode.com
[1]: https://github.com/darrenpmeyer/pyStrangerBoard

## Implementation notes

This sketch was built for a [DFRobot Beetle][2], a small and inexpensive Arduino Leonardo clone. However, any Arduino-compatible
board supported by [FastLED 3.1][3] should work.

I targetted a WS2811-compatible string (WS2812 and WS2812-B should work without changes as well) of LEDs with an RGB ordering;
you can change this inside `setup()` by changing the `FastLED.addLeds` call to match your string.

Basically, if you connect a WS2811-compatible LED string's data line to pin D9, this should work without changes.

Configuration for the sketch is done with the top `#define` statements. You should only need to change the `LED_DATA_PIN` if you're
using similar lights to mine.

[2]: https://www.dfrobot.com/index.php?route=product/product&product_id=1075
[3]: https://fastled.io
