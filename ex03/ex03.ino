// 定义 LED 引脚
const int ledPins[] = {2, 4, 18};
const int numLeds = 3;

// 时序定义 (毫秒)
const unsigned long SHORT_ON = 200;   // 短闪亮灯时间
const unsigned long LONG_ON = 600;    // 长闪亮灯时间
const unsigned long OFF_TIME = 200;   // 闪灭之间的间隔
const unsigned long SOS_PAUSE = 1500; // SOS 整体播放完后的长停顿

// 状态枚举
enum State {
  STATE_S_1_ON, STATE_S_1_OFF,
  STATE_S_2_ON, STATE_S_2_OFF,
  STATE_S_3_ON, STATE_S_3_OFF,
  STATE_O_1_ON, STATE_O_1_OFF,
  STATE_O_2_ON, STATE_O_2_OFF,
  STATE_O_3_ON, STATE_O_3_OFF,
  STATE_S_4_ON, STATE_S_4_OFF, // 这里的 S 代表 SOS 的最后一组
  STATE_S_5_ON, STATE_S_5_OFF,
  STATE_S_6_ON, STATE_S_6_OFF,
  STATE_LONG_PAUSE
};

State currentState = STATE_S_1_ON; // 初始状态
unsigned long previousMillis = 0;  // 记录上次时间

void setup() {
  // 初始化所有 LED 引脚
  for (int i = 0; i < numLeds; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }
}

void loop() {
  unsigned long currentMillis = millis();

  // 检查是否到达了执行下一步的时间
  if (currentMillis - previousMillis >= getDuration(currentState)) {
    // 保存当前时间
    previousMillis = currentMillis;

    // 状态流转逻辑
    switch (currentState) {
      // --- S (3 Short) ---
      case STATE_S_1_ON: digitalWriteAll(HIGH); currentState = STATE_S_1_OFF; break;
      case STATE_S_1_OFF: digitalWriteAll(LOW); currentState = STATE_S_2_ON; break;
      case STATE_S_2_ON: digitalWriteAll(HIGH); currentState = STATE_S_2_OFF; break;
      case STATE_S_2_OFF: digitalWriteAll(LOW); currentState = STATE_S_3_ON; break;
      case STATE_S_3_ON: digitalWriteAll(HIGH); currentState = STATE_S_3_OFF; break;
      case STATE_S_3_OFF: digitalWriteAll(LOW); currentState = STATE_O_1_ON; break;

      // --- O (3 Long) ---
      case STATE_O_1_ON: digitalWriteAll(HIGH); currentState = STATE_O_1_OFF; break;
      case STATE_O_1_OFF: digitalWriteAll(LOW); currentState = STATE_O_2_ON; break;
      case STATE_O_2_ON: digitalWriteAll(HIGH); currentState = STATE_O_2_OFF; break;
      case STATE_O_2_OFF: digitalWriteAll(LOW); currentState = STATE_O_3_ON; break;
      case STATE_O_3_ON: digitalWriteAll(HIGH); currentState = STATE_O_3_OFF; break;
      case STATE_O_3_OFF: digitalWriteAll(LOW); currentState = STATE_S_4_ON; break;

      // --- S (3 Short) ---
      case STATE_S_4_ON: digitalWriteAll(HIGH); currentState = STATE_S_4_OFF; break;
      case STATE_S_4_OFF: digitalWriteAll(LOW); currentState = STATE_S_5_ON; break;
      case STATE_S_5_ON: digitalWriteAll(HIGH); currentState = STATE_S_5_OFF; break;
      case STATE_S_5_OFF: digitalWriteAll(LOW); currentState = STATE_S_6_ON; break;
      case STATE_S_6_ON: digitalWriteAll(HIGH); currentState = STATE_S_6_OFF; break;
      case STATE_S_6_OFF: digitalWriteAll(LOW); currentState = STATE_LONG_PAUSE; break;

      // --- Long Pause ---
      case STATE_LONG_PAUSE: currentState = STATE_S_1_ON; break;
    }
  }
}

// 辅助函数：获取当前状态需要持续的时间
unsigned long getDuration(State state) {
  switch (state) {
    // 亮灯时间
    case STATE_S_1_ON: case STATE_S_2_ON: case STATE_S_3_ON:
    case STATE_S_4_ON: case STATE_S_5_ON: case STATE_S_6_ON:
      return SHORT_ON;

    case STATE_O_1_ON: case STATE_O_2_ON: case STATE_O_3_ON:
      return LONG_ON;

    // 灭灯时间（包括符号间隔和最后的长停顿）
    case STATE_S_1_OFF: case STATE_S_2_OFF:
    case STATE_S_4_OFF: case STATE_S_5_OFF:
    case STATE_O_1_OFF: case STATE_O_2_OFF:
      return OFF_TIME;

    case STATE_S_3_OFF: // S 和 O 之间的间隔
    case STATE_O_3_OFF: // O 和 S 之间的间隔
      return OFF_TIME; 

    case STATE_LONG_PAUSE:
      return SOS_PAUSE;
      
    default:
      return 0;
  }
}

// 辅助函数：同时控制所有 LED
void digitalWriteAll(int value) {
  for (int i = 0; i < numLeds; i++) {
    digitalWrite(ledPins[i], value);
  }
}