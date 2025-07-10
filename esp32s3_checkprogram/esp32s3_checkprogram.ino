#include <WiFi.h>
#include <FastLED.h>
#include <WebServer.h>
#include <ESPmDNS.h>

#define BOARD_NAME    "esp32s3board"
#define MDNS_HOSTNAME "esp32s3board"
#define WIFI_AP_SSID  "ESP32S3-AP"
#define WIFI_AP_PSK   "12345678"

constexpr int PIN_LED = 21;
constexpr int pin_gpio_count = 10;
const int pin_gpio[pin_gpio_count] = { 0, 1, 3, 5, 7, 9, 11, 13, 15 };

WebServer httpServer(80);

CRGB leds[1];

void blink_led(unsigned int interval_msec) {
  static bool led_on = false;
  static unsigned long timer = 0;
  if (timer == 0) {
    FastLED.addLeds<WS2812B, PIN_LED, GRB>(leds, 1);
    FastLED.clear();
    FastLED.show();
  }
  const unsigned long now = millis();
  if (now - timer >= interval_msec) {
    timer = now;
    led_on = !led_on;
    leds[0] = led_on ? CRGB(32, 32, 32) : CRGB(0, 0, 0);
    FastLED.show();
  }
}


void handleRoot() {
  httpServer.send(200, "text/html", "<h1>Hello, world!</h1><p>from " BOARD_NAME " board!</p>");
}


void task_gpio_wave() {
  static int gpio_index = 0;
  static unsigned long timer = 0;
  const unsigned long interval = 200;
  const unsigned long now = millis();
  if (timer == 0) {
    for (int i = 0; i < pin_gpio_count; i++) {
      pinMode(pin_gpio[i], OUTPUT);
    }
  }
  if (now - timer >= interval) {
    timer = now;
    digitalWrite(pin_gpio[gpio_index], LOW);
    gpio_index = (gpio_index + 1) % pin_gpio_count;
    digitalWrite(pin_gpio[gpio_index], HIGH);
  }
}

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 3000 && !Serial; i++) {
    delay(1);
    blink_led(100);
  }

  Serial.println("Initializing...");
  
  blink_led(0);
  leds[0] = CRGB(64, 0, 0);
  FastLED.show();
  delay(500);
  leds[0] = CRGB(0, 64, 0);
  FastLED.show();
  delay(500);
  leds[0] = CRGB(0, 0, 64);
  FastLED.show();
  delay(500);
  FastLED.clear();
  FastLED.show();

  Serial.println("Starting WiFi AP...");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PSK);

  Serial.println("Waiting connection on " WIFI_AP_SSID " with password " WIFI_AP_PSK);

  Serial.println("Starting HTTP server...");
  httpServer.on("/", handleRoot);
  httpServer.onNotFound(handleRoot);
  httpServer.begin();

  Serial.println("Starting MDNS...");
  MDNS.begin(MDNS_HOSTNAME);

  Serial.println("Listening on port 80 of " MDNS_HOSTNAME);

  Serial.println("Ready.");
}

void loop() {
  httpServer.handleClient();
  blink_led(500);
  task_gpio_wave();
}
