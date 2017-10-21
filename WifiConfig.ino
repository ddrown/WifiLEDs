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

void callbackREST(AsyncWebServerRequest *request) {
	//its possible to test the url and do different things, 
	//test you rest URL
	if (request->url() == "/rest/userdemo")	{
    FSInfo fs_info;
		//contruct and send and desired repsonse
		// get sample data from json file
		String data = "";
		ESPHTTPServer.load_user_config("user1", data);
		String values = "user1|"+ data +"|input\n";

		ESPHTTPServer.load_user_config("user2", data);
		values += "user2|" + data + "|input\n";

		ESPHTTPServer.load_user_config("user3", data);
		values += "user3|" + data + "|input\n";

    values += "chipsize|" + String(ESP.getFlashChipRealSize()) + "|div\n";
    values += "sketchsize|" + String(ESP.getSketchSize()) + "|div\n";
    values += "freespace|" + String(ESP.getFreeSketchSpace()) + "|div\n";

    SPIFFS.info(fs_info);
    values += "fstotal|" + String(fs_info.totalBytes) + "|div\n";
    values += "fsused|" + String(fs_info.usedBytes) + "|div\n";
    values += "fsblock|" + String(fs_info.blockSize) + "|div\n";
    values += "fspage|" + String(fs_info.pageSize) + "|div\n";
    values += "fsmaxopen|" + String(fs_info.maxOpenFiles) + "|div\n";
    values += "fsmaxpath|" + String(fs_info.maxPathLength) + "|div\n";

		request->send(200, "text/plain", values);
		values = "";
	}	else { 
		//its possible to test the url and do different things, 
		String values = "message:Hello world! \nurl:" + request->url() + "\n";
		request->send(200, "text/plain", values);
		values = "";
	}
}

void callbackPOST(AsyncWebServerRequest *request) {
	//its possible to test the url and do different things, 
	if (request->url() == "/post/user")	{
		String target = "/";

		for (uint8_t i = 0; i < request->args(); i++) {
      DEBUGLOG("Arg %d: %s\r\n", i, request->arg(i).c_str());
			Serial.print(request->argName(i));
			Serial.print(" : ");
			Serial.println(ESPHTTPServer.urldecode(request->arg(i)));

			//check for post redirect
			if (request->argName(i) == "afterpost") {
				target = ESPHTTPServer.urldecode(request->arg(i));
			} else {
				ESPHTTPServer.save_user_config(request->argName(i), request->arg(i));
			}
    }

		request->redirect(target);
	} else if(request->url() == "/post/led")  {
    int brightness = 0;
    String values = "LED";
    for (uint8_t i = 0; i < request->args(); i++) {
      if (request->argName(i) == "brightness") {
        brightness = request->arg(i).toInt();
        FastLED.setBrightness(brightness);
        values = values + " set to brightness " + String(brightness);
      }
    }
    values = values + "\n";
    request->send(200, "text/plain", values);
    values = "";
	}	else {
		String values = "message:Hello world! \nurl:" + request->url() + "\n";
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

#define BRIGHTNESS         255
#define FRAMES_PER_SECOND  120

void setup() {
  // WiFi is started inside library
  SPIFFS.begin(); // Not really needed, checked inside library and started if needed
  ESPHTTPServer.begin(&SPIFFS);

	//set optional callback
	ESPHTTPServer.setRESTCallback(callbackREST);

	//set optional callback
 
	ESPHTTPServer.setPOSTCallback(callbackPOST);
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
}

uint8_t gHue = 0; // rotating "base color" used by many of the patterns

void loop() {
  bpm();

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
