
// 定义引脚
const int ledPinA = 5;  // 连接到引脚 9 (支持 PWM ~9)
const int ledPinB = 19; // 连接到引脚 10 (支持 PWM ~10)

// 定义变量
int brightness = 0;    // 初始亮度
int fadeAmount = 5;    // 亮度变化的步长 (数值越小变化越细腻，但速度变慢)

void setup() {
  // 初始化引脚模式
  pinMode(ledPinA, OUTPUT);
  pinMode(ledPinB, OUTPUT);
}

void loop() {
  // 1. 设置 LED A 的亮度 (从 0 增加到 255)
  analogWrite(ledPinA, brightness);

  // 2. 设置 LED B 的亮度 (反相逻辑：从 255 减小到 0)
  // 核心算法：B 的亮度 = 最大值 - A 的亮度
  analogWrite(ledPinB, 255 - brightness);

  // 3. 计算下一次的亮度
  brightness = brightness + fadeAmount;

  // 4. 判断是否到达边界 (0 或 255)，如果到达则反转变化方向
  if (brightness <= 0 || brightness >= 255) {
    fadeAmount = -fadeAmount;
  }

  // 5. 短暂延迟，让人眼能观察到平滑的渐变
  delay(30); 
}