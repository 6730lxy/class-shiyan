// 定义LED引脚数组
const int ledPins[] = {2, 4, 18};
const int ledCount = 3; // LED数量

// PWM设置
const int freq = 5000;
const int resolution = 8;

void setup() {
  Serial.begin(115200);
  
  // 批量初始化引脚
  for(int i = 0; i < ledCount; i++){
    ledcAttach(ledPins[i], freq, resolution);
  }
}

void loop() {
  // 依次遍历每个LED
  for(int i = 0; i < ledCount; i++){
    
    // --- 渐亮 ---
    for(int dutyCycle = 0; dutyCycle <= 255; dutyCycle++){   
      ledcWrite(ledPins[i], dutyCycle);   
      delay(5); // 稍微调快一点速度，否则三个灯跑完一圈有点久
    }

    // --- 渐暗 ---
    for(int dutyCycle = 255; dutyCycle >= 0; dutyCycle--){
      ledcWrite(ledPins[i], dutyCycle);   
      delay(5);
    }
    
    Serial.print("LED on Pin ");
    Serial.print(ledPins[i]);
    Serial.println(" cycle completed");
  }
}