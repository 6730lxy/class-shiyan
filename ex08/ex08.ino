#include <WiFi.h>
#include <WebServer.h>

// ================= 配置区域 =================
const char* ssid = "流浪汉";
const char* password = "hello1234world";

// 硬件引脚定义
const int ledPin = 2;           // 板载 LED
const int touchPin = 4;         // 触摸引脚 T0 (GPIO 4)

// 系统状态变量
bool isArmed = false;           // 系统是否布防 (默认撤防)
bool isAlarmTriggered = false;  // 是否已触发报警 (锁定状态)

// 触摸灵敏度 (数值越小越灵敏，通常在 20-50 之间调试)
const uint16_t touchThreshold = 40; 

WebServer server(80);

// ================= 网页 HTML 代码 =================
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>物联网安防报警器</title>
  <style>
    body { font-family: 'Arial', sans-serif; text-align: center; margin-top: 50px; background-color: #222; color: white; }
    h1 { margin-bottom: 30px; }
    .status-box { padding: 20px; border-radius: 10px; margin: 20px auto; width: 80%; max-width: 300px; font-size: 1.2em; font-weight: bold; }
    
    /* 状态颜色 */
    .safe { background-color: #28a745; box-shadow: 0 0 15px #28a745; } /* 绿色 */
    .armed { background-color: #ffc107; color: #333; box-shadow: 0 0 15px #ffc107; } /* 黄色 */
    .alarm { background-color: #dc3545; box-shadow: 0 0 20px #dc3545; animation: pulse 0.5s infinite; } /* 红色闪烁 */

    /* 按钮样式 */
    .btn { padding: 15px 30px; font-size: 1.2em; margin: 10px; border: none; border-radius: 5px; cursor: pointer; color: white; width: 80%; max-width: 200px; }
    .btn-arm { background-color: #ffc107; color: #333; }
    .btn-disarm { background-color: #dc3545; }
    
    @keyframes pulse { 0% { opacity: 1; } 50% { opacity: 0.6; } 100% { opacity: 1; } }
  </style>
</head>
<body>
  <h1>🛡️ 智能安防主机</h1>
  
  <!-- 状态显示区域 -->
  <div id="statusBox" class="status-box safe">
    系统状态: 已撤防 (安全)
  </div>

  <!-- 控制按钮 -->
  <div>
    <button class="btn btn-arm" onclick="setMode('arm')">🔒 布防 (Arm)</button>
    <button class="btn btn-disarm" onclick="setMode('disarm')">🔓 撤防 (Disarm)</button>
  </div>

  <p style="color: #888; margin-top: 30px; font-size: 0.8em;">触摸 GPIO 4 进行测试</p>

  <script>
    // 定时刷新状态 (每 500ms 一次)
    setInterval(checkStatus, 500);

    function checkStatus() {
      fetch('/status')
        .then(response => response.json())
        .then(data => {
          const box = document.getElementById("statusBox");
          if (data.alarm) {
            box.className = "status-box alarm";
            box.innerHTML = "🚨 报警！入侵检测！";
          } else if (data.armed) {
            box.className = "status-box armed";
            box.innerHTML = "🛡️ 已布防 (警戒中)";
          } else {
            box.className = "status-box safe";
            box.innerHTML = "✅ 已撤防 (安全)";
          }
        });
    }

    function setMode(mode) {
      fetch('/control?mode=' + mode)
        .then(response => response.text())
        .then(data => console.log(data));
    }
  </script>
</body>
</html>
)rawliteral";

// ================= 处理函数 =================

// 1. 主页
void handleRoot() {
  server.send(200, "text/html", index_html);
}

// 2. 接收控制指令 (布防/撤防)
void handleControl() {
  if (server.hasArg("mode")) {
    String mode = server.arg("mode");
    if (mode == "arm") {
      isArmed = true;
      // 布防时，如果之前有报警，可以选择是否重置，这里我们假设布防即重置报警状态
      isAlarmTriggered = false; 
      digitalWrite(ledPin, LOW); // 关灯
      Serial.println("系统已布防");
    } else if (mode == "disarm") {
      isArmed = false;
      isAlarmTriggered = false; // 撤防清除报警锁定
      digitalWrite(ledPin, LOW); // 强制关灯
      Serial.println("系统已撤防");
    }
    server.send(200, "text/plain", "OK");
  }
}

// 3. 前端轮询状态
void handleStatus() {
  // 构建 JSON 返回给前端
  String json = "{\"armed\":" + String(isArmed ? "true" : "false") + 
                ", \"alarm\":" + String(isAlarmTriggered ? "true" : "false") + "}";
  server.send(200, "application/json", json);
}

// ================= 主程序 =================

void setup() {
  Serial.begin(115200);
  
  // LED 初始化
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); // 初始关闭

  // WiFi 连接
  WiFi.begin(ssid, password);
  Serial.print("连接 WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n连接成功");
  Serial.print("IP: http://");
  Serial.println(WiFi.localIP());

  // 路由设置
  server.on("/", handleRoot);
  server.on("/control", handleControl);
  server.on("/status", handleStatus);
  
  server.begin();
}

void loop() {
  server.handleClient(); // 处理网页请求

  // --- 安防核心逻辑 ---
  
  // 1. 读取触摸值
  int touchValue = touchRead(touchPin);
  // Serial.println(touchValue); // 调试时可打开，观察触摸数值变化

  // 2. 判断逻辑
  // 只有当系统处于“布防”状态 且 “未处于报警锁定”状态 时，才检测触摸
  if (isArmed && !isAlarmTriggered) {
    // 如果触摸值小于阈值 (手触碰时电容变化导致数值变小)
    if (touchValue < touchThreshold) {
      isAlarmTriggered = true; // 触发锁定
      Serial.println("触发报警！");
    }
  }

  // 3. 执行报警动作
  if (isAlarmTriggered) {
    // 高频闪烁 (模拟警报)
    digitalWrite(ledPin, HIGH);
    delay(100); // 亮 100ms
    digitalWrite(ledPin, LOW);
    delay(100); // 灭 100ms
    // 注意：这里的 delay 会轻微阻塞网页响应，但在单任务实验中是可接受的
  } else {
    // 未报警时，确保灯是灭的 (除非你有其他逻辑)
    // 注意：不要在这里加 delay，否则网页加载会很慢
    // 我们只在报警循环里控制灯，这里保持熄灭状态即可
    if(digitalRead(ledPin) == HIGH) digitalWrite(ledPin, LOW);
  }
  
  delay(10); // 小延时防止看门狗报错
}