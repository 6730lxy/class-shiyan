#define TOUCH_PIN 4    // 推荐使用 GPIO4，它是最稳定的触摸引脚之一
#define LED_PIN 2      // 板载 LED 通常在 GPIO2

// 状态变量
bool ledState = false; // LED 当前状态
unsigned long lastTouchTime = 0; // 记录上一次触发的时间
const unsigned long DEBOUNCE_DELAY = 300; // 消抖时间：300ms (防止手抖误触)

// 阈值：注意！不同环境阈值差异很大，建议先通过串口监视器观察数值
// 通常静止时读数较大(如80-100)，触摸时读数变小(如30-50)
// 如果你的环境干扰大，可能需要调整此值
#define THRESHOLD 50 

// 中断服务函数 (ISR)
// IRAM_ATTR 宏确保函数在 IRAM 中运行，提高响应速度
void IRAM_ATTR gotTouch() {
  unsigned long currentTime = millis();
  
  // 软件防抖逻辑
  if (currentTime - lastTouchTime > DEBOUNCE_DELAY) {
    // 翻转状态
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
    
    // 更新时间戳
    lastTouchTime = currentTime;
    
    // 串口打印调试信息 (可选)
    Serial.println(ledState ? "👉 LED 开启 (ON)" : "👈 LED 关闭 (OFF)");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // 初始化 LED 为灭
  
  Serial.println("🚀 系统启动中...");
  
  // 绑定中断
  // 参数说明：引脚, 回调函数, 阈值(读数低于此值触发中断)
  // 注意：ESP32 的 touchAttachInterrupt 第三个参数是阈值
  touchAttachInterrupt(TOUCH_PIN, gotTouch, THRESHOLD);
  
  Serial.println("✅ 触摸传感器已就绪，请触摸 GPIO4...");
}

void loop() {
  // 主循环可以处理其他任务，例如读取其他传感器
  // 因为使用了中断，这里不需要写任何检测代码
}