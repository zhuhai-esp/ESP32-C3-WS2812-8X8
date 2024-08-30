#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <WiFi.h>

#define PIN_LED1 12
#define PIN_LED2 13

// Matrix Data PIN
#define PIN_PIXS 1
#define PIX_NUM 64

Adafruit_NeoPixel pixels(PIX_NUM, PIN_PIXS, NEO_GRB + NEO_KHZ800);

long check1s = 0, check10ms = 0, check300ms = 0;

uint32_t fill_colors[] = {0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00,
                          0x00FFFF, 0xFF00FF, 0xFFFFFF};

void inline initBoard() {
  pinMode(PIN_LED1, OUTPUT);
  pinMode(PIN_LED2, OUTPUT);
  digitalWrite(PIN_LED1, LOW);
  digitalWrite(PIN_LED2, LOW);
  pixels.begin();
  pixels.setBrightness(16);
  pixels.clear();
  pixels.show();
}

void inline autoConfigWifi() {
  pixels.setPixelColor(0, fill_colors[0]);
  pixels.show();
  WiFi.mode(WIFI_MODE_STA);
  WiFi.begin();
  for (int i = 0; WiFi.status() != WL_CONNECTED && i < 100; i++) {
    delay(100);
  }
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_MODE_APSTA);
    WiFi.beginSmartConfig();
    while (WiFi.status() != WL_CONNECTED) {
      delay(100);
    }
    WiFi.stopSmartConfig();
    WiFi.mode(WIFI_MODE_STA);
  }
  pixels.setPixelColor(0, fill_colors[1]);
  pixels.show();
}

void inline startConfigTime() {
  const int timeZone = 8 * 3600;
  configTime(timeZone, 0, "ntp6.aliyun.com", "cn.ntp.org.cn", "ntp.ntsc.ac.cn");
  while (time(nullptr) < 8 * 3600 * 2) {
    delay(500);
  }
  pixels.setPixelColor(0, fill_colors[2]);
  pixels.show();
}

void inline setupOTAConfig() {
  ArduinoOTA.onStart([] {});
  ArduinoOTA.onProgress([](u32_t pro, u32_t total) {});
  ArduinoOTA.onEnd([] {});
  ArduinoOTA.onError([](ota_error_t err) {});
  ArduinoOTA.begin();
}

void inline pixelsCheck() {
  for (uint32_t c : fill_colors) {
    pixels.fill(c);
    pixels.show();
    delay(500);
  }
}

void setup() {
  initBoard();
  pixelsCheck();
  autoConfigWifi();
  setupOTAConfig();
  startConfigTime();
  pixels.clear();
  pixels.show();
}

void loop() {
  auto ms = millis();
  if (ms - check1s > 1000) {
    check1s = ms;
    ArduinoOTA.handle();
  }
  if (ms - check300ms > 300) {
    check300ms = ms;
  }
  if (ms - check10ms >= 10) {
    check10ms = ms;
  }
}
