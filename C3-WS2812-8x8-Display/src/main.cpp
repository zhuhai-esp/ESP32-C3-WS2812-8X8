#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <WiFi.h>

#define PIN_LED1 12
#define PIN_LED2 13

#define PIN_PIXS 1
#define PIX_NUM 64

// 定义数字的点阵数据
const uint32_t digits[] = {
    0x4200000, //        0B00000100001000000000000000000000, // .
    0x0,
    0x69999996, //        0B01101001100110011001100110010110, // 0
    0xE4444464, //        0B11100100010001000100010001100100, // 1
    0xF1248896, //        0B11110001001001001000100010010110, // 2
    0x69886896, //        0B01101001100010000110100010010110, // 3
    0x444F5564, //        0B01000100010011110101010101100100, // 4
    0x6988871F, //        0B01101001100010001000011100011111, // 5
    0x69997196, //        0B01101001100110010111000110010110, // 6
    0x4444888F, //        0B01000100010001001000100010001111, // 7
    0x69996996, //        0B01101001100110010110100110010110, // 8
    0x6988E996, //        0B01101001100010001110100110010110  // 9
    0x200200,   //        0B00000000001000000000001000000000, // :
};

Adafruit_NeoPixel pixels(PIX_NUM, PIN_PIXS, NEO_GRB + NEO_KHZ800);

long check1s = 0, check10ms = 0, check300ms = 0;

uint32_t fill_colors[] = {0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00,
                          0x00FFFF, 0xFF00FF, 0xFFFFFF};

char buf[256] = {0};
u64_t secCount = 0;

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
  ArduinoOTA.onStart([] {
    pixels.clear();
    pixels.show();
  });
  ArduinoOTA.onProgress([](u32_t pro, u32_t total) {
    uint8_t count = pro * 64 / total;
    pixels.clear();
    for (uint8_t i = 0; i < count; i++) {
      pixels.setPixelColor(i, fill_colors[i % 5]);
    }
    pixels.show();
  });
  ArduinoOTA.onEnd([] {
    pixels.clear();
    pixels.show();
  });
  ArduinoOTA.onError([](ota_error_t err) {});
  ArduinoOTA.begin();
}

void inline pixelsCheck() {
  for (uint32_t c : fill_colors) {
    pixels.fill(c);
    pixels.show();
    delay(500);
  }
  pixels.clear();
  pixels.show();
}

void inline scrollText(String txt) {
  pixels.clear();
  for (int32_t h = 0; h < txt.length() * 4 + 2; h++) {
    for (uint8_t i = 0; i < txt.length(); i++) {
      auto c = txt.charAt(i) - 46;
      if (c >= 0 && c <= 12) {
        auto d = digits[c];
        for (uint8_t j = 0; j < 8; j++) {
          for (uint8_t k = 0; k < 4; k++) {
            if (0x1 & (d >> (j * 4 + k))) {
              int32_t off = i * 4 + k - h + 8;
              if (off >= 0 && off < 8) {
                u8_t offset = j * 8 + off;
                pixels.setPixelColor(offset, fill_colors[i % 5]);
              }
            }
          }
        }
      }
    }
    pixels.show();
    delay(200);
    pixels.clear();
    ArduinoOTA.handle();
  }
}

void inline showIPAddress() {
  auto ip = WiFi.localIP().toString();
  for (uint8_t i = 0; i < 2; i++) {
    scrollText(ip);
  }
  sleep(3);
}

inline void showCurrentTime() {
  struct tm info;
  for (uint8_t i = 0; i < 2; i++) {
    getLocalTime(&info);
    strftime(buf, 36, "%T", &info);
    scrollText(buf);
  }
  sleep(3);
}

void setup() {
  initBoard();
  pixelsCheck();
  autoConfigWifi();
  setupOTAConfig();
  startConfigTime();
}

void loop() {
  auto ms = millis();
  if (ms - check1s > 1000) {
    check1s = ms;
    ArduinoOTA.handle();
    secCount += 1;
    if (secCount % 60 == 1) {
      showCurrentTime();
      showIPAddress();
    }
  }
  if (ms - check300ms > 300) {
    check300ms = ms;
  }
  if (ms - check10ms >= 10) {
    check10ms = ms;
    pixels.rainbow((check10ms << 4) % 0xFFFF);
    pixels.show();
  }
}
