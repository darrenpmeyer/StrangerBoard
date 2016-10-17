#include <FastLED.h>

/** Configuration **/
#define LED_COUNT 50
#define LED_DATA_PIN 9      // data pin for WS281x data+clock line
#define TIME_LETTERON 500   // ms to leave lights on for blink_letter
#define TIME_LETTEROFF 300  // ms to leave blank between letters
#define TIME_BOARDOFF 1800  // ms to leave board blank between messages


CRGB led[LED_COUNT];
uint8_t lightmap[27];
//CRGBPalette16 letterPalette;

#define Blank fill_solid(led, LED_COUNT, CRGB::Black)
#define arraysize(x) sizeof(x)/sizeof(x[0])

/** load the "Stranger Things" inspired LED color map into program memory, it shouldn't change
 *  The lines are long because each row maps to a row in the Stranger Things version of the board
 */
const CRGB stranger_map[27] = {
  CRGB::White, CRGB::Blue, CRGB::Magenta, CRGB::Red, CRGB::Blue, CRGB::Yellow, CRGB::Magenta, CRGB::Blue,
  CRGB::Blue, CRGB::Magenta, CRGB::Blue, CRGB::Green, CRGB::Yellow, CRGB::Magenta, CRGB::Magenta, CRGB::Blue, CRGB::Magenta,
  CRGB::Green, CRGB::White, CRGB::Yellow, CRGB::Blue, CRGB::Magenta, CRGB::Blue, CRGB::Yellow, CRGB::Magenta, CRGB::Magenta,
  CRGB::Red
};


String message[] = { "HACKATHON X", "STRANGER HACKS", "THAT IS NO MOON" };

/** setup() - run once at Arduino init, sets up lightmap and FastLED
 *  
 *  Opens Serial port at 9600, 8-N-1
 *  Sets lightmap for a board that runs as follows:
 *    1 row, left-to-right, 9 lights (A-I); skip 1 light
 *    1 row, right-to-left, 9 lights (J-R, reversed); skip 1 light
 *    1 row, left-to-right, 9 lights (S-Z and space)
 *   
 *  Current configuration does _not_ use the last light on the board for
 *  the space char, but the last light of the _string_. You can change this by
 *  commenting out the relevant assignment below. You might want that if you
 *  want space char to be easily visible on a string with more than 27 lights.
 */
void setup() {
  delay(1500);  // for interrupting during startup
  Serial.begin(9600);
  
  FastLED.addLeds<WS2812, LED_DATA_PIN, RGB>(led, LED_COUNT);
  Blank;
  FastLED.show();

  // set default light pattern
  for (uint8_t i=0; i < 27; i++) {
    lightmap[i] = i;
  }

  lightmap[17] = 10;
  lightmap[16] = 11;
  lightmap[15] = 12;
  lightmap[14] = 13;
  lightmap[13] = 14;
  lightmap[12] = 15;
  lightmap[11] = 16;
  lightmap[10] = 17;
  lightmap[9] = 18;

  lightmap[18] = 20;
  for (uint8_t i = 19; i < 27; i++) {
    lightmap[i] = lightmap[i-1] + 1;
  }

  /* light the very last one for space... keeps it off the board.
   * Comment it out to use the last light _on the board_ for space */
  lightmap[26] = LED_COUNT-1;  

//  letterPalette = RainbowColors_p;
}


/** get_clor(light) - gets the color for a particular light number (not string index!)
 * 
 *  Uses the right colors for a given letter from the Stranger Things show
 *  see https://i.imgur.com/VJapZLS.jpg
 * 
 *  return: CRGB struct for the color to use. Called from blink_letter()
 */
CRGB get_color(uint8_t light) {
  return stranger_map[light];
//  return ColorFromPalette(letterPalette, 255/light, 240, LINEARBLEND);
}


/** blink_letter(letter) - causes a letter to blink in the right place on the string
 * 
 *  consumes the globals 'led' and 'lightmap' to make the right LED blink.
 */
void blink_letter(char letter) {
  uint8_t cval = letter;
  uint8_t light = 26;  // by default, light up the "Space" light

  Serial.print(letter);
  
  if (cval == 32) {
    light = 26; // leave alone, is space
  }
  else if (cval < 65 || cval > 90) {
    return; // skip it, not A-Z or space
  }
  else {
    light = cval-65; // change A-Z to 0-25
  }

  // Now blink the right light
  led[lightmap[light]] = get_color(light);  // light this light from the lightmap
  FastLED.show();
  FastLED.delay(TIME_LETTERON);
  Blank;
  FastLED.show();
}


/** blink_message(msg) - blinks a messsage one char at a time
 * 
 *  Calls blink_letter for each char
 * 
 *  return false if the message fails (e.g. too short), true otherwise
 */
bool blink_message(String msg) {
  msg.trim();
  if (msg.length() > 1) {
    for (uint8_t i = 0; i < msg.length(); i++) {
      blink_letter(msg.charAt(i));
      FastLED.delay(TIME_LETTEROFF); // Leave the board blank between letters
    }
    Serial.println("");
    return true;
  }

  return false;
}


/** buffer_message() - buffers message from serial input
 *  
 *  Reads a string terminated by \n, upper-cases it, and pushes it to the end
 *  of the global message[] array, shifting other messages up (FIFO).
 *  
 *  return 0 if success, < 0 for errors
 *  
 *  errors: 
 *    return -1 if message is too long (>100 chars) or too short (<3 chars)
 */
int buffer_message() {
  String newmessage;

  while(Serial.available()) {
    newmessage = Serial.readStringUntil('\n');
    newmessage.toUpperCase();
    newmessage.trim();
  }

  if (newmessage.length() > 100 || newmessage.length() < 3) {
    return -1;  // message too long or short
    //TODO more precice errors
  }
  
  for (uint8_t i = 1; i < arraysize(message); i++) {
    message[i-1] = message[i];
  }
  message[arraysize(message)-1] = newmessage;

  return 0;
}

/** errorflash(times, gap, color) - flash the whole string to indicate a status
 *  
 *  this should probably be "statusflash", but eh.
 *  
 *  Blinks the string 'color', 'times' times, with each on/off cycle lasting 'gap' ms.
 *  
 *  So errorflash(5, 100, CRGB::Blue) would flash the string blue 5 times; each flash would be
 *  50ms on and 50ms off.
 */
void errorflash(uint8_t times, unsigned int gap, CRGB color) {
  Blank;
  
  for (uint8_t i = 0; i < times; i++) {
    fill_solid(led, LED_COUNT, color);
    FastLED.show();
    FastLED.delay(gap/2);
    Blank;
    FastLED.show();
    FastLED.delay(gap/2);
  }
}


/** loop() - main Arduino event loop, called repeatedly
 *  
 *  Cycles through each item in message[] and displays it using 'blink_message'
 *  
 *  If there is serial data available between messages, adds it to the queue;
 *  blinks green twice when new message is inserted, red 5 times if there's an
 *  error inserting it.
 *  
 *  Because the serial interrupt is processed between messages, only the last message
 *  sent during display will be queued. Clients have to wait for a queue confirmation
 *  before sending the next message.
 */
void loop() {
  static uint8_t m_idx = 0;

  if (Serial.available()) {
    if (buffer_message() < 0) {  // add the message to the end of the message buffer
      // error state: string too short or too long
      errorflash(5, 100, CRGB::Red);
      Serial.println("Message size error");
    }
    else {
      m_idx = arraysize(message)-1;  // tee it up to be displayed next
      Serial.print("Queued message '"); Serial.print(message[m_idx]); Serial.print("'");
      Serial.println("");
      errorflash(2, 50, CRGB::Green);
    }
  }
  
  if (blink_message(message[m_idx])) { FastLED.delay(TIME_BOARDOFF); } // delay between messages, only if message was displayed

  m_idx++;
  if (m_idx >= arraysize(message)) { m_idx = 0; }
}
