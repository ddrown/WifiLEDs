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

struct LEDconfig {
  uint8_t pattern;
  CRGB static_color;
  uint8_t brightness;
} settings = {PATTERN_BPM, CRGB::Black, 255};

uint8_t gHue = 0;

void callbackREST(AsyncWebServerRequest *request) {
	if (request->url() == "/rest/user")	{
    uint32_t color;
    String color_s;
		String values = "pattern|"+ String(settings.pattern) +"|input\n";

    color = settings.static_color[0];
    color = color << 8 | settings.static_color[1];
    color = color << 8 | settings.static_color[2];
    color_s = String(color, HEX);
    while(color_s.length() < 6) {
      color_s = "0"+color_s;
    }
		values += "static_color|#" + color_s + "|input\n";

    values += "brightness|" + String(settings.brightness) + "|input\n";

		request->send(200, "text/plain", values);
		values = "";
	}	else {
		String values = "message:Default Response\nurl:" + request->url() + "\n";
		request->send(200, "text/plain", values);
		values = "";
	}
}

void callbackPOST(AsyncWebServerRequest *request) {
	if (request->url() == "/post/user")	{
		String target = "/";

		for (uint8_t i = 0; i < request->args(); i++) {
      if (request->argName(i) == "brightness") {
        settings.brightness = request->arg(i).toInt();
        FastLED.setBrightness(settings.brightness);
        ESPHTTPServer.save_user_config(request->argName(i), request->arg(i));
        
      } else if (request->argName(i) == "pattern") {
        settings.pattern = request->arg(i).toInt();
        ESPHTTPServer.save_user_config(request->argName(i), request->arg(i));
        
      } else if (request->argName(i) == "static_color") {
        const char *color = request->arg(i).c_str();
        if(color[0] == '#') {
          int32_t new_color = strtol(color+1, 0, 16);
          if(new_color >= 0 && new_color < 16777216) {
            settings.static_color = (uint32_t)new_color;
            ESPHTTPServer.save_user_config("static_color", String(new_color));
          }
        }
      }
    }

		request->redirect(target);
	}	else {
		String values = "message:Default Post\nurl:" + request->url() + "\n";
		request->send(200, "text/plain", values);
		values = "";
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
    
  ESPHTTPServer.load_user_config("static_color", data);
  if(data.length() > 0)
    settings.static_color = data.toInt();
    
  ESPHTTPServer.load_user_config("brightness", data);
  if(data.length() > 0)
    settings.brightness = data.toInt();
}

void setup() {
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
  
  CRGBPalette16 palette = RainbowColors_p;
  for( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette(palette, gHue+i, sin8(i*4-fadestep));
  }
}

void static_pattern() {
  for( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = settings.static_color;
  } 
}

void walk_pattern() {  
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = settings.static_color;
    leds[i] %= sin8(gHue+i);
  } 
}

void fade_pattern() {
  CRGB faded_color = settings.static_color;
  faded_color %= sin8(gHue);
  
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = faded_color;
  } 
}

