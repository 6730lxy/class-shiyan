// --- 1. 引脚与参数定义 ---
#define LED_PIN     2       // LED引脚 (板载LED通常是GPIO2)
#define TOUCH_PIN   4       // 触摸引脚 (T0 对应 GPIO4)

// PWM 设置
const int freq = 5000;      // 频率 5000Hz
const int resolution = 8;   // 分辨率 8位 (0-255)

// --- 2. 全局变量 ---
int speedLevel = 1;         // 速度档位：1=慢, 2=中, 3=快
int delayTime = 10;         // 延时时间，根据档位动态变化
int touchThreshold = 20;    // 触摸阈值 (根据串口监视器数据调整)

void setup() {
  Serial.begin(115200);
  
  // 初始化 LED PWM
  // 新版 ledcAttach(引脚, 频率, 分辨率) 写法
  ledcAttach(LED_PIN, freq, resolution);
  
  Serial.println("系统就绪，请触摸 GPIO4 切换速度...");
}

void loop() {
  // --- 3. 核心逻辑：呼吸循环 ---
  
  // 第一阶段：渐亮 (0 -> 255)
  for(int dutyCycle = 0; dutyCycle <= 255; dutyCycle++){   
    ledcWrite(LED_PIN, dutyCycle);
    
    // 【关键点】在循环内部检测触摸，而不是等呼吸完再检测
    // 这样切换速度时不会有明显的卡顿感
    checkTouchAndChangeSpeed();
    
    delay(delayTime); // 使用变量延时，而不是固定的10
  }

  // 第二阶段：渐暗 (255 -> 0)
  for(int dutyCycle = 255; dutyCycle >= 0; dutyCycle--){
    ledcWrite(LED_PIN, dutyCycle);
    
    // 同样在渐暗过程中检测触摸
    checkTouchAndChangeSpeed();
    
    delay(delayTime);
  }
}

// --- 4. 辅助函数：处理触摸逻辑 ---
void checkTouchAndChangeSpeed() {
  // 读取触摸值
  int touchValue = touchRead(TOUCH_PIN);
  
  // 简单的消抖：连续读取两次确认（可选，防止误触）
  // 这里为了响应灵敏，直接使用单次读取判断
  
  if (touchValue < touchThreshold) {
    // 检测到触摸 (ESP32 触摸值通常越小代表越接近)
    
    // 稍微延时防止一次触摸触发多次跳变
    delay(200); 
    
    // 切换档位逻辑：1 -> 2 -> 3 -> 1
    speedLevel++;
    if (speedLevel > 3) {
      speedLevel = 1;
    }
    
    // 根据档位更新延时时间
    switch(speedLevel) {
      case 1: 
        delayTime = 15; // 慢速呼吸
        Serial.println("档位 1: 慢速"); 
        break;
      case 2: 
        delayTime = 8;  // 中速呼吸
        Serial.println("档位 2: 中速"); 
        break;
      case 3: 
        delayTime = 2;  // 快速呼吸
        Serial.println("档位 3: 快速"); 
        break;
    }
    
    // 等待手指离开，防止连续触发（简单的阻塞式消抖）
    while(touchRead(TOUCH_PIN) < touchThreshold) {
      delay(50);
    }
  }
}