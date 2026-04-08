#include <WiFi.h>
#include <WebServer.h>

// ================= 配置区域 =================
const char* ssid = "流浪汉";
const char* password = "hello1234world";

// 硬件定义
const int touchPin = 4;         // 触摸引脚 T0 (GPIO 4)
const int ledPin = 2;           // 板载 LED (用于指示数据请求)

WebServer server(80);

// ================= 网页 HTML 代码 (前端) =================
// 包含 CSS 样式和 AJAX 脚本
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 实时仪表盘</title>
  <style>
    body { 
      font-family: 'Segoe UI', sans-serif; 
      background-color: #1a1a1a; 
      color: white; 
      text-align: center; 
      padding-top: 50px; 
    }
    .dashboard-card {
      background-color: #2d2d2d;
      width: 80%;
      max-width: 400px;
      margin: 0 auto;
      border-radius: 15px;
      padding: 30px;
      box-shadow: 0 10px 25px rgba(0,0,0,0.5);
    }
    h2 { margin-top: 0; color: #00d2ff; }
    
    /* 数字显示样式 */
    .sensor-value {
      font-size: 4em;
      font-weight: bold;
      margin: 20px 0;
      font-variant-numeric: tabular-nums; /* 防止数字跳动时宽度变化 */
    }
    
    /* 进度条样式 */
    .progress-bg {
      background-color: #444;
      border-radius: 10px;
      height: 20px;
      width: 100%;
      overflow: hidden;
    }
    .progress-fill {
      background: linear-gradient(90deg, #00d2ff, #3a7bd5);
      height: 100%;
      width: 0%; /* 初始宽度为0 */
      transition: width 0.1s ease-out; /* 平滑过渡动画 */
    }
    
    .status { margin-top: 20px; font-size: 0.8em; color: #888; }
  </style>
</head>
<body>

  <div class="dashboard-card">
    <h2>📡 触摸传感器监控</h2>
    
    <!-- 实时数值显示 -->
    <div id="valueDisplay" class="sensor-value">0</div>
    
    <!-- 可视化进度条 -->
    <div class="progress-bg">
      <div id="barDisplay" class="progress-fill"></div>
    </div>
    
    <div class="status">数据每 200ms 刷新一次</div>
  </div>

  <script>
    // AJAX 核心逻辑
    function fetchSensorData() {
      // 向 ESP32 请求数据接口
      fetch('/api/data')
        .then(response => response.json()) // 解析 JSON 数据
        .then(data => {
          // 更新网页内容
          const val = data.touch_value;
          document.getElementById("valueDisplay").innerText = val;
          
          // 更新进度条宽度 (假设最大值约 100，根据实际情况调整)
          // 这里做一个简单的映射，让视觉效果更明显
          let percentage = (val / 100) * 100; 
          if(percentage > 100) percentage = 100;
          document.getElementById("barDisplay").style.width = percentage + "%";
        })
        .catch(error => console.error('获取数据失败:', error));
    }

    // 设置定时器，每 200 毫秒执行一次 fetchSensorData
    setInterval(fetchSensorData, 200);
  </script>
</body>
</html>
)rawliteral";

// ================= 处理函数 =================

// 1. 主页处理：发送 HTML 页面
void handleRoot() {
  server.send(200, "text/html", index_html);
}

// 2. 数据接口处理：发送 JSON 数据
void handleApiData() {
  // 读取触摸传感器的原始数值
  uint16_t touchValue = touchRead(touchPin);
  
  // 闪烁 LED 指示数据正在被读取 (可选，方便调试)
  digitalWrite(ledPin, HIGH);
  delay(1); 
  digitalWrite(ledPin, LOW);

  // 构建 JSON 字符串
  // 格式: {"touch_value": 45}
  String json = "{\"touch_value\":" + String(touchValue) + "}";
  
  // 发送 JSON 响应
  server.send(200, "application/json", json);
}

// ================= 主程序 =================

void setup() {
  Serial.begin(115200);
  
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // 连接 WiFi
  WiFi.begin(ssid, password);
  Serial.print("连接 WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\n连接成功");
  Serial.print("访问地址: http://");
  Serial.println(WiFi.localIP());

  // 设置路由
  server.on("/", handleRoot);       // 访问网页
  server.on("/api/data", handleApiData); // 访问数据接口
  
  server.begin();
}

void loop() {
  server.handleClient();
}