#include <WiFi.h>
#include <WebServer.h>

// ================= 配置区域 =================
const char* ssid = "流浪汉";     // 请修改为你的 WiFi 名称
const char* password = "hello1234world"; // 请修改为你的 WiFi 密码

// 定义 LED 引脚 (根据你的呼吸灯程序，这里使用 GPIO 2)
const int ledPin = 2;  

// PWM 属性设置
const int freq = 5000;          // 频率 5000Hz (避免低频闪烁)
const int resolution = 8;       // 分辨率 8位 (0-255)

// 初始化 Web 服务器 (端口 80)
WebServer server(80);

// ================= 网页 HTML 代码 =================
// 这里包含了滑动条样式和 JavaScript 脚本
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 无极调光器</title>
  <style>
    body { font-family: 'Arial', sans-serif; text-align: center; margin-top: 50px; background-color: #f4f4f4; }
    h1 { color: #333; }
    .slider-container { margin: 20px auto; width: 80%; max-width: 400px; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 4px 8px rgba(0,0,0,0.1); }
    /* 滑动条样式美化 */
    input[type=range] { width: 100%; cursor: pointer; }
    .value-display { font-size: 1.5em; font-weight: bold; color: #007BFF; margin-top: 10px; display: block; }
  </style>
</head>
<body>
  <h1>💡 ESP32 无极调光器</h1>
  
  <div class="slider-container">
    <p>拖动滑块调节 LED 亮度</p>
    <!-- 滑动条：范围 0-255，对应 PWM 占空比 -->
    <input type="range" min="0" max="255" value="0" id="brightnessSlider" oninput="updateBrightness(this.value)">
    <span id="sliderValue" class="value-display">0</span>
  </div>

  <script>
    // JavaScript 函数：当滑动条变动时触发
    function updateBrightness(val) {
      // 1. 更新页面上显示的数值
      document.getElementById("sliderValue").innerText = val;
      
      // 2. 使用 fetch 发送 GET 请求给 ESP32
      // 请求格式: /light?brightness=0 到 /light?brightness=255
      fetch('/light?brightness=' + val)
        .then(response => console.log('指令已发送: ' + val))
        .catch(error => console.error('发送失败:', error));
    }
  </script>
</body>
</html>
)rawliteral";

// ================= 处理函数 =================

// 1. 处理主页请求 ("/")
void handleRoot() {
  server.send(200, "text/html", index_html);
}

// 2. 处理亮度调节请求 ("/light")
void handleLight() {
  // 检查 URL 中是否包含 brightness 参数
  if (server.hasArg("brightness")) {
    // 获取参数值并转换为整数
    int brightness = server.arg("brightness").toInt();
    
    // 安全限制：确保数值在 0-255 之间
    brightness = constrain(brightness, 0, 255);
    
    // 【核心操作】将数值写入 PWM 通道
    // 使用你提供的 ledcWrite 写法，直接控制引脚
    ledcWrite(ledPin, brightness);
    
    Serial.print("亮度已调整为: ");
    Serial.println(brightness);
    
    // 返回成功状态给浏览器
    server.send(200, "text/plain", "OK");
  } else {
    // 如果没有参数，返回错误
    server.send(400, "text/plain", "Bad Request");
  }
}

// ================= 主程序 =================

void setup() {
  Serial.begin(115200);

  // 配置 PWM
  // 对应你提供的呼吸灯程序中的 ledcAttach
  ledcAttach(ledPin, freq, resolution);
  // 初始关闭 LED
  ledcWrite(ledPin, 0); 

  // 连接 WiFi
  WiFi.begin(ssid, password);
  Serial.print("正在连接 WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi 连接成功!");
  Serial.print("请在浏览器访问: http://");
  Serial.println(WiFi.localIP());

  // 设置路由规则
  server.on("/", handleRoot);      // 访问主页
  server.on("/light", handleLight); // 接收调光指令
  
  server.begin();
  Serial.println("HTTP 服务器已启动");
}

void loop() {
  // 持续处理客户端请求
  server.handleClient();
}