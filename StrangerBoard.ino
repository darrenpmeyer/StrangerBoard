#include <FastLED.h>

#define LED_COUNT 50

CRGB led[LED_COUNT];
uint8_t lightmap[27];
CRGBPalette16 letterPalette;

#define Blank fill_solid(led, LED_COUNT, CRGB::Black)
#define arraysize(x) sizeof(x)/sizeof(x[0])


String message[] = { "HACKATHON X", "STRANGER HACKS", "THAT IS NO MOON" };

void setup() {
  delay(1500);  // for interrupting during startup
  Serial.begin(9600);
  
  FastLED.addLeds<WS2812, 9, RGB>(led, LED_COUNT); // Pin 9, RGB light order
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

  letterPalette = PartyColors_p;
}


CRGB get_color(uint8_t light) {
  return ColorFromPalette(letterPalette, 255/light, 240, LINEARBLEND);
}


void blink_letter(char letter) {
  uint8_t cval = letter;
  uint8_t light = 26;  // by default, light up the "Space" light
//  CRGB color = CRGB::Blue;

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
  delay(300);

  // Now blink the right light
  led[lightmap[light]] = get_color(light);  // light this light from the lightmap
  FastLED.show();
  FastLED.delay(300);
  Blank;
  FastLED.show();
}

bool blink_message(String msg) {
  msg.trim();
  if (msg.length() > 1) {
    for (uint8_t i = 0; i < msg.length(); i++) {
      blink_letter(msg.charAt(i));
      FastLED.delay(300);  // 300ms of darkness
    }
    Serial.println("");
    return true;
  }

  return false;
}

int buffer_message() {
  String newmessage;

  while(Serial.available()) {
    newmessage = Serial.readStringUntil('\n');
    newmessage.toUpperCase();
    newmessage.trim();
//    Serial.print("New message: ");
//    Serial.println(newmessage);
  }

  if (newmessage.length() > 100 || newmessage.length() < 3) {
    Serial.print("Message "); Serial.print(newmessage);
    Serial.print(" too long or short ("); Serial.print(newmessage.length()); Serial.print(" chars)");
    Serial.println();
    delay(300); 
    return -1;  // message too long or short
    //TODO more precice errors
  }
  
  for (uint8_t i = 1; i < arraysize(message); i++) {
    message[i-1] = message[i];
  }
  message[arraysize(message)-1] = newmessage;

  return 0;
}

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
  
  if (blink_message(message[m_idx])) { FastLED.delay(1200); } // delay between messages, only if message was displayed

  m_idx++;
  if (m_idx >= arraysize(message)) { m_idx = 0; }
}
