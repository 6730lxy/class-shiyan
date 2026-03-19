// 定义 LED 连接的引脚为 2
const int ledPin = 2; 

// 定义闪烁间隔：1Hz 频率 = 1秒周期 = 亮500ms + 灭500ms
const unsigned long interval = 500; 

// 用于记录上次 LED 状态改变的时间
unsigned long previousMillis = 0; 

// 用于存储 LED 的当前状态
int ledState = LOW; 

void setup() {
  // 将引脚 2 设置为输出模式
  pinMode(ledPin, OUTPUT);
}

void loop() {
  // 获取当前时间（毫秒）
  unsigned long currentMillis = millis();

  // 检查是否已经过了设定的时间间隔
  if (currentMillis - previousMillis >= interval) {
    // 保存当前时间，作为下一次计时的起点
    previousMillis = currentMillis;

    // 切换 LED 的状态
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }

    // 将新的状态写入引脚 2
    digitalWrite(ledPin, ledState);
  }
}