#include <Adafruit_NeoPixel.h>

// --- 灯带配置 ---
#define STRIP1_PIN 6
#define STRIP2_PIN 5
#define TOTAL_LEDS 60
#define ACTIVE_LEDS 50

Adafruit_NeoPixel strip1(TOTAL_LEDS, STRIP1_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2(TOTAL_LEDS, STRIP2_PIN, NEO_GRB + NEO_KHZ800);

// --- Flex 传感器配置 ---
const int flexPin1 = A1;
float smoothF1 = 0;
float filterWeight = 0.1;

// --- 定时器 ---
unsigned long lastSensorRead = 0;
unsigned long lastLED = 0;

// --- 灯带动画偏移 ---
static float ledOffset = 0;

void setup() {
  Serial.begin(9600);

  strip1.begin();
  strip2.begin();
  strip1.setBrightness(40);
  strip2.setBrightness(40);
  strip1.show();
  strip2.show();

  smoothF1 = analogRead(flexPin1);

  Serial.println("Flex + LED strip only");
}

void loop() {
  unsigned long now = millis();

  // --- 1. 读取并平滑 flex 数据 ---
  if (now - lastSensorRead >= 20) {
    lastSensorRead = now;

    int rawF1 = analogRead(flexPin1);
    smoothF1 = (smoothF1 * (1.0 - filterWeight)) + (rawF1 * filterWeight);

    Serial.print("Raw Flex: ");
    Serial.print(rawF1);
    Serial.print(" | Smooth Flex: ");
    Serial.println((int)smoothF1);
  }

  // --- 2. 根据 flex 值刷新灯带 ---
  if (now - lastLED > 20) {
    unsigned long delta = now - lastLED;
    lastLED = now;

    // 根据 flex 值调节滚动速度
    float scrollSpeed = (smoothF1 < 30) ? 0.0015 :
                        (smoothF1 < 50) ? 0.0022 : 0.005;

    // 高 flex 时闪烁
    bool strobe = (smoothF1 >= 50) ? ((now / 70) % 2 == 0) : true;

    ledOffset += scrollSpeed * delta * 10.0;
    if (ledOffset > 6.0) ledOffset -= 6.0;

    for (int i = 0; i < ACTIVE_LEDS; i++) {
      // 灯带1：白色流动效果
      if (!strobe) {
        strip1.setPixelColor(i, strip1.Color(2, 2, 2));
      } else {
        float pos = fmod((float)i + ledOffset, 6.0);
        strip1.setPixelColor(i,
          (pos < 2.0) ? strip1.Color(180, 180, 180) : strip1.Color(5, 5, 5));
      }

      // 灯带2：根据 flex 强度变化红色亮度
      if (smoothF1 < 30) {
        strip2.setPixelColor(i, strip2.Color(20, 0, 0));
      } else if (smoothF1 < 50) {
        strip2.setPixelColor(i, strip2.Color(80, 0, 0));
      } else {
        strip2.setPixelColor(i, strip2.Color(255, 0, 0));
      }
    }

    strip1.show();
    strip2.show();
  }
}