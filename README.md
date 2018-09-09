Libraries:

  * FSBrowserNG / Browser config web interface - https://github.com/ddrown/FSBrowserNG
  * ESPAsyncWebServer / async web server - https://github.com/me-no-dev/ESPAsyncWebServer
  * ESPAsyncTCP / Async TCP sockets - https://github.com/me-no-dev/ESPAsyncTCP
  * ArduinoJson / JSON parser - https://github.com/bblanchon/ArduinoJson
  * FastLED / RGB LED library - https://github.com/FastLED/FastLED

Example video: https://www.youtube.com/watch?v=r6tvzuMIBck

To upload the data/ directory to your esp8266:

  * you'll need the ESP8266 filesystem uploader extension - https://github.com/esp8266/arduino-esp8266fs-plugin
  * Tools > ESP8266 Sketch Data Upload
  * there's also a web interface to modify the webserver content, as well as an OTA firmware update

ws2811 data is on esp8266 pin D5

NUM\_LEDS is the total # of LEDs to output, set at 100 by default

Default HTTP user/pass is in data/secret.json.  On first boot, it will create an AP with the name "ESPxxxx" where xxxx is the chip id.  The AP WPA password is the same as the HTTP password.  You can set the SSID and password to join through the admin web interface at http://192.168.1.4/admin.html  The HTTP user/pass should also be changed there.

curl.sh has some examples of controlling this interface through curl (for example, from a raspberry pi).  I'm relying on my dhcp server to setup the hostname esp8266fs.lan.  If your dhcp server can't do this, you can use either mdns or the IP.
