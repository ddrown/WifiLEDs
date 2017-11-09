#include <ESP8266WiFi.h>
#include "FS.h"
#include <WiFiClient.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266mDNS.h>
#include <Ticker.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include "FSWebServerLib.h"
#include <Hash.h>
#include "FastLED.h"

#define PATTERN_BPM 0
#define PATTERN_STATIC 1
#define PATTERN_WALK 2
#define PATTERN_FADE 3
#define PATTERN_TWINKLE 4
#define PATTERN_PALETTE 5

#define STATIC_COLOR_COUNT 16
struct LEDconfig {
  uint8_t pattern;
  uint8_t brightness;
  CRGB static_colors[STATIC_COLOR_COUNT];
} settings = {pattern: PATTERN_BPM, brightness: 255};

uint8_t gHue = 0;

void callbackREST(AsyncWebServerRequest *request) {
	if (request->url() == "/rest/user")	{
		String values = "pattern|"+ String(settings.pattern) +"|input\n";

    for(uint8_t i = 0; i < STATIC_COLOR_COUNT; i++) {
      uint32_t color;
      String color_s;
      
      color = settings.static_colors[i][0];
      color = color << 8 | settings.static_colors[i][1];
      color = color << 8 | settings.static_colors[i][2];
      color_s = String(color, HEX);
      while(color_s.length() < 6) {
        color_s = "0"+color_s;
      }

		  values += "static_color_"+String(i)+"|#" + color_s + "|input\n";
    }

    values += "brightness|" + String(settings.brightness) + "|input\n";

    values += "compile_date|" __DATE__ " " __TIME__ "|div\n";

		request->send(200, "text/plain", values);
	}	else {
		String values = "message:Default Response\nurl:" + request->url() + "\n";
		request->send(200, "text/plain", values);
	}
}

void callbackPOST(AsyncWebServerRequest *request) {
	if (request->url() == "/post/user")	{
		for (uint8_t i = 0; i < request->args(); i++) {
      if (request->argName(i) == "brightness") {
        settings.brightness = request->arg(i).toInt();
        FastLED.setBrightness(settings.brightness);
        ESPHTTPServer.save_user_config(request->argName(i), request->arg(i));
        
      } else if (request->argName(i) == "pattern") {
        settings.pattern = request->arg(i).toInt();
        ESPHTTPServer.save_user_config(request->argName(i), request->arg(i));
        
      } else if (request->argName(i).length() >= 14 && request->argName(i).length() <= 16 && request->argName(i).startsWith("static_color_")) {
        String colorid_s = request->argName(i).substring(13);
        int colorid = colorid_s.toInt();
        if(colorid >= 0 && colorid < STATIC_COLOR_COUNT) {
          const char *color = request->arg(i).c_str();
          if(color[0] == '#') {
            int32_t new_color = strtol(color+1, 0, 16);
            if(new_color >= 0 && new_color < 16777216) {
              settings.static_colors[colorid] = (uint32_t)new_color;
              ESPHTTPServer.save_user_config("static_color_"+colorid_s, String(new_color));
            }
          }
        }
      }
    }

		request->redirect("/");
	}	else {
		String values = "message:Default Post\nurl:" + request->url() + "\n";
		request->send(200, "text/plain", values);
	}
}

FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    5
#define LED_TYPE    WS2811
#define COLOR_ORDER RGB
#define NUM_LEDS    100
CRGB leds[NUM_LEDS];

#define FRAMES_PER_SECOND  120

void load_user_config() {
  String data = "";
  
  ESPHTTPServer.load_user_config("pattern", data);
  if(data.length() > 0)
    settings.pattern = data.toInt();

  for(uint8_t i = 0; i < STATIC_COLOR_COUNT; i++) {
    ESPHTTPServer.load_user_config("static_color_"+String(i), data);
    if(data.length() > 0)
      settings.static_colors[i] = data.toInt();
  }
    
  ESPHTTPServer.load_user_config("brightness", data);
  if(data.length() > 0)
    settings.brightness = data.toInt();
}

void setup() {
  for(uint8_t i = 0; i < STATIC_COLOR_COUNT; i++) {
    settings.static_colors[i] = CRGB::Black;
  }
  
  // WiFi is started inside library
  SPIFFS.begin(); // Not really needed, checked inside library and started if needed
  ESPHTTPServer.begin(&SPIFFS);

	ESPHTTPServer.setRESTCallback(callbackREST);
	ESPHTTPServer.setPOSTCallback(callbackPOST);
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(Typical8mmPixel);

  load_user_config();
  
  FastLED.setBrightness(settings.brightness);
}

void loop() {
  switch(settings.pattern) {
    case PATTERN_STATIC:
      static_pattern();
      break;
    case PATTERN_WALK:
      walk_pattern();
      break;
    case PATTERN_FADE:
      fade_pattern();
      break;
    case PATTERN_TWINKLE:
      twinkle_pattern();
      break;
    case PATTERN_PALETTE:
      palette_pattern();
      break;
    case PATTERN_BPM:
    default:
      bpm();
      break;
  }

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  
  unsigned long start = millis();
  do {
    delay(1);
    ESPHTTPServer.handle();
  } while((millis()-start) < (1000/FRAMES_PER_SECOND));

  // do some periodic updates
  EVERY_N_MILLISECONDS( 40 ) { gHue++; } // slowly cycle the "base color" through the rainbow
}

void bpm() {
  static uint8_t fadestep = 0;
  fadestep++;
  
  CRGBPalette16 palette(
    settings.static_colors[0],
    settings.static_colors[1],
    settings.static_colors[2],
    settings.static_colors[3],
    settings.static_colors[4],
    settings.static_colors[5],
    settings.static_colors[6],
    settings.static_colors[7],
    settings.static_colors[8],
    settings.static_colors[9],
    settings.static_colors[10],
    settings.static_colors[11],
    settings.static_colors[12],
    settings.static_colors[13],
    settings.static_colors[14],
    settings.static_colors[15]
    );
  for( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette(palette, gHue+i, sin8(i*4-fadestep));
  }
}

void palette_pattern() {
  CRGBPalette16 palette(
    settings.static_colors[0],
    settings.static_colors[1],
    settings.static_colors[2],
    settings.static_colors[3],
    settings.static_colors[4],
    settings.static_colors[5],
    settings.static_colors[6],
    settings.static_colors[7],
    settings.static_colors[8],
    settings.static_colors[9],
    settings.static_colors[10],
    settings.static_colors[11],
    settings.static_colors[12],
    settings.static_colors[13],
    settings.static_colors[14],
    settings.static_colors[15]
    );
  for( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette(palette, gHue+i, 255);
  }
}

void static_pattern() {
  for( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = settings.static_colors[0];
  } 
}

void walk_pattern() {  
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = settings.static_colors[0];
    leds[i] %= sin8(gHue+i);
  } 
}

void fade_pattern() {
  CRGB faded_color = settings.static_colors[0];
  faded_color %= sin8(gHue);
  
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = faded_color;
  } 
}

void twinkle_pattern() {
  static uint8_t last_gHue = 0, last_led = 0, flipflop = 0;

  if((uint8_t)(gHue-last_gHue) >= 25) { // every 1000ms (25*40ms)
    last_gHue = gHue;
    last_led = secureRandom(NUM_LEDS/4);
    flipflop = (flipflop + 1) % 12;
  }

  if(flipflop == 5 || flipflop == 11) {
    for(int i = 0; i < NUM_LEDS; i++) {
      uint8_t mod_i = i % (NUM_LEDS/4);
      CRGB c1, c2;
      if(flipflop == 5) {
        if(i % 2) {
          c2 = settings.static_colors[0];
          c1 = settings.static_colors[1];
        } else {
          c1 = settings.static_colors[0];
          c2 = settings.static_colors[1];
        }
      } else {
        if(i % 2) {
          c1 = settings.static_colors[0];
          c2 = settings.static_colors[1];
        } else {
          c2 = settings.static_colors[0];
          c1 = settings.static_colors[1];
        }
      }
      leds[i] = blend(c1, c2, (gHue-last_gHue)*10);
      
      if(mod_i == last_led) {
        leds[i] %= cos8((gHue-last_gHue)*10);
      }     
    }
    return;
  }

  for(int i = 0; i < NUM_LEDS; i++) {
    uint8_t mod_i = i % (NUM_LEDS/4);
    
    if(flipflop >= 5) { // every ~5 seconds
      leds[i] = (i % 2) ? settings.static_colors[0] : settings.static_colors[1];
    } else {
      leds[i] = (i % 2) ? settings.static_colors[1] : settings.static_colors[0];
    }
    if(mod_i == last_led) {
      leds[i] %= cos8((gHue-last_gHue)*10);
    }
  }
}
