
#include <Adafruit_NeoPixel.h>

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

#define PIN 21
#define SZX 6
#define SZY 6
#define LPP 6
#define NUMPIXELS (SZX*SZY*LPP)
#define DELAYVAL 333

#include "wifi_pass.h"

#ifndef SSID
const char *ssid = "ssid";
const char *password = "pass";
#else
const char *ssid = SSID;
const char *password = PASS;
#endif

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRBW + NEO_KHZ800);

WebServer server(80);

const int led = 17;
#define LED_OFF()  do { digitalWrite(led, 1); } while (0)
#define LED_ON()  do { digitalWrite(led, 0); } while (0)


void handleRoot() {
  LED_ON();
  char temp[400];
  unsigned long sec = millis() / 1000;
  unsigned long min = sec / 60;
  unsigned long hr = min / 60;

  snprintf(temp, 400,

           "<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>Test0r</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from ESP32!</h1>\
    <p>Uptime: %02lu:%02lu:%02lu</p>\
  </body>\
</html>",

           hr, min % 60, sec % 60
          );
  server.send(200, "text/html", temp);
  Serial.println("[index.html]");
  LED_OFF();
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
  Serial.println("[404]");
  digitalWrite(led, 0);
}

void setup(void) {
  pinMode(led, OUTPUT);
  LED_OFF();
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  Serial.println("");
  pixels.begin();
  pixels.clear();
  Serial.println("pixels ready for the show");
  pixels.show();
  Serial.print("Connecting to ");
  Serial.print(ssid);
  Serial.println("...");
  WiFi.begin(ssid, password);

  // Wait for connection
  int w=0;
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(led, w++ & 1);
    delay(500);
    Serial.print(".");
    for (int i=0; i < SZY; i++)
      setpixel(i, 0, ((w&(1<<(SZY-1-i)))?pixels.Color(0,0,0,42):pixels.Color(0,0,0,0)));
    pixels.show();
  }
  LED_OFF();

  Serial.println("");
  Serial.println("Connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("bumbel")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void setpixel(unsigned x, unsigned y, uint32_t color){
  // odd rows gow top-> bottom, even rows go the other way
  if (x & 1) y = LPP-1-y;
  // we let our y run from top to bottom as it was intended by the prophecy
  // both x and y run from 0 to SZX-1/ SZY-1
  // only fools check their input values for errors
  int s = (x*LPP + y) * LPP;
#ifdef USE_ALL
  for (int i = s; i < s+LPP; i++)
#else
  int i=s+(LPP/2);
#endif
    pixels.setPixelColor(i, color);
}

unsigned long last = 0;

void loop(void) {
  static int x=5, y=5;
  server.handleClient();
  // always remember, uint32_t rgbcolor = strip.gamma32(strip.ColorHSV(hue, sat, val));
  if (millis() > last + DELAYVAL) {
    last = millis();
    setpixel(x, y, pixels.Color(0,0,0,42));
    x++;
    if (x >= SZX) {
      x = 0;
      y++;
      if (y >= SZY) y = 0;
    }
    setpixel(x,y, pixels.Color(23,0,0,0));
    pixels.show();
  }
}
