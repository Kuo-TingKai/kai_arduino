/*
 * ESP32-CAM Interactive Console
 * Enhanced version with WiFi, Camera, Bluetooth, and Web Interface
 * 
 * Features:
 * - WiFi Access Point and Station modes
 * - Camera streaming and photo capture
 * - Web-based control interface
 * - Serial command interface
 * - Bluetooth communication
 * - All original Arduino console functions
 * 
 * Hardware: ESP32-CAM with ESP32-CAM-MB programmer board
 */

#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <BluetoothSerial.h>
#include <EEPROM.h>
#include <Wire.h>

// Camera pin definitions for AI Thinker ESP32-CAM
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Available GPIO pins for user functions
#define LED_BUILTIN        4  // Built-in LED (with flash)
#define LED_STATUS        33  // Status LED
#define USER_GPIO_12      12
#define USER_GPIO_13      13
#define USER_GPIO_14      14
#define USER_GPIO_15      15
#define USER_GPIO_16      16

// Network configuration
const char* ap_ssid = "ESP32-CAM-Console";
const char* ap_password = "12345678";
const char* hostname = "esp32cam";

// Global objects
WebServer server(80);
BluetoothSerial SerialBT;
String inputString = "";
bool stringComplete = false;
bool cameraInitialized = false;
bool wifiConnected = false;
String connectedSSID = "";

// Communication flags
bool useSerial = true;
bool useBluetooth = false;
bool useWebSocket = false;

void setup() {
  // Initialize Serial communication
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  
  // Initialize EEPROM
  EEPROM.begin(512);
  
  // Initialize I2C
  Wire.begin();
  
  // Initialize GPIO pins
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_STATUS, OUTPUT);
  pinMode(USER_GPIO_12, INPUT_PULLUP);
  pinMode(USER_GPIO_13, INPUT_PULLUP);
  pinMode(USER_GPIO_14, OUTPUT);
  pinMode(USER_GPIO_15, OUTPUT);
  pinMode(USER_GPIO_16, OUTPUT);
  
  // Initialize SPIFFS for web files
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS initialization failed!");
  }
  
  // Initialize camera
  initCamera();
  
  // Initialize WiFi
  initWiFi();
  
  // Initialize Bluetooth
  SerialBT.begin("ESP32-CAM-Console");
  
  // Initialize web server
  initWebServer();
  
  // Print welcome message
  printWelcomeMessage();
  
  // Startup blink
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_STATUS, HIGH);
    delay(200);
    digitalWrite(LED_STATUS, LOW);
    delay(200);
  }
}

void loop() {
  // Handle web server
  server.handleClient();
  
  // Handle Serial input
  if (Serial.available()) {
    useSerial = true;
    useBluetooth = false;
    while (Serial.available()) {
      char inChar = (char)Serial.read();
      if (inChar == '\n' || inChar == '\r') {
        if (inputString.length() > 0) {
          processCommand(inputString);
          inputString = "";
        }
        printPrompt();
      } else {
        inputString += inChar;
      }
    }
  }
  
  // Handle Bluetooth input
  if (SerialBT.available()) {
    useSerial = false;
    useBluetooth = true;
    while (SerialBT.available()) {
      char inChar = (char)SerialBT.read();
      if (inChar == '\n' || inChar == '\r') {
        if (inputString.length() > 0) {
          processCommand(inputString);
          inputString = "";
        }
        printPrompt();
      } else {
        inputString += inChar;
      }
    }
  }
  
  // Status LED heartbeat
  static unsigned long lastHeartbeat = 0;
  if (millis() - lastHeartbeat > 2000) {
    digitalWrite(LED_STATUS, !digitalRead(LED_STATUS));
    lastHeartbeat = millis();
  }
  
  delay(10); // Small delay to prevent watchdog timeout
}

void initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  // Higher quality for smaller image sizes
  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // Initialize camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    cameraInitialized = false;
    return;
  }
  
  cameraInitialized = true;
  Serial.println("Camera initialized successfully");
  
  // Adjust camera settings
  sensor_t* s = esp_camera_sensor_get();
  s->set_brightness(s, 0);     // -2 to 2
  s->set_contrast(s, 0);       // -2 to 2
  s->set_saturation(s, 0);     // -2 to 2
  s->set_special_effect(s, 0); // 0 to 6 (0-No Effect, 1-Negative, 2-Grayscale, 3-Red Tint, 4-Green Tint, 5-Blue Tint, 6-Sepia)
  s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
  s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
  s->set_wb_mode(s, 0);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
  s->set_exposure_ctrl(s, 1);  // 0 = disable , 1 = enable
  s->set_aec2(s, 0);           // 0 = disable , 1 = enable
  s->set_ae_level(s, 0);       // -2 to 2
  s->set_aec_value(s, 300);    // 0 to 1200
  s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
  s->set_agc_gain(s, 0);       // 0 to 30
  s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
  s->set_bpc(s, 0);            // 0 = disable , 1 = enable
  s->set_wpc(s, 1);            // 0 = disable , 1 = enable
  s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
  s->set_lenc(s, 1);           // 0 = disable , 1 = enable
  s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
  s->set_vflip(s, 0);          // 0 = disable , 1 = enable
  s->set_dcw(s, 1);            // 0 = disable , 1 = enable
  s->set_colorbar(s, 0);       // 0 = disable , 1 = enable
}

void initWiFi() {
  // Start Access Point mode
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ap_ssid, ap_password);
  
  Serial.println("WiFi Access Point started");
  Serial.print("AP SSID: ");
  Serial.println(ap_ssid);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  
  // Set hostname
  WiFi.setHostname(hostname);
  
  // Initialize mDNS
  if (MDNS.begin(hostname)) {
    MDNS.addService("http", "tcp", 80);
    Serial.printf("mDNS responder started: http://%s.local\n", hostname);
  }
}

void initWebServer() {
  // Serve main page
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", getMainPage());
  });
  
  // Camera stream
  server.on("/stream", HTTP_GET, handleCameraStream);
  
  // Camera capture
  server.on("/capture", HTTP_GET, handleCameraCapture);
  
  // API endpoints
  server.on("/api/status", HTTP_GET, handleAPIStatus);
  server.on("/api/command", HTTP_POST, handleAPICommand);
  server.on("/api/wifi/scan", HTTP_GET, handleWiFiScan);
  server.on("/api/wifi/connect", HTTP_POST, handleWiFiConnect);
  
  // GPIO control
  server.on("/api/gpio/read", HTTP_GET, handleGPIORead);
  server.on("/api/gpio/write", HTTP_POST, handleGPIOWrite);
  
  server.begin();
  Serial.println("Web server started");
}

void handleCameraStream() {
  if (!cameraInitialized) {
    server.send(503, "text/plain", "Camera not initialized");
    return;
  }
  
  WiFiClient client = server.client();
  
  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
  server.sendContent(response);
  
  while (client.connected()) {
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      break;
    }
    
    String header = "--frame\r\n";
    header += "Content-Type: image/jpeg\r\n";
    header += "Content-Length: " + String(fb->len) + "\r\n\r\n";
    
    server.sendContent(header);
    client.write(fb->buf, fb->len);
    server.sendContent("\r\n");
    
    esp_camera_fb_return(fb);
    
    if (!client.connected()) break;
    delay(30); // ~30 FPS
  }
}

void handleCameraCapture() {
  if (!cameraInitialized) {
    server.send(503, "text/plain", "Camera not initialized");
    return;
  }
  
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    server.send(500, "text/plain", "Camera capture failed");
    return;
  }
  
  server.sendHeader("Content-Disposition", "inline; filename=capture.jpg");
  server.send_P(200, "image/jpeg", (const char*)fb->buf, fb->len);
  
  esp_camera_fb_return(fb);
}

void handleAPIStatus() {
  DynamicJsonDocument doc(1024);
  
  doc["uptime"] = millis();
  doc["freeHeap"] = ESP.getFreeHeap();
  doc["chipModel"] = ESP.getChipModel();
  doc["chipRevision"] = ESP.getChipRevision();
  doc["flashSize"] = ESP.getFlashChipSize();
  doc["camera"] = cameraInitialized;
  doc["wifi"]["mode"] = WiFi.getMode();
  doc["wifi"]["ap_ip"] = WiFi.softAPIP().toString();
  doc["wifi"]["connected"] = WiFi.isConnected();
  if (WiFi.isConnected()) {
    doc["wifi"]["sta_ip"] = WiFi.localIP().toString();
    doc["wifi"]["ssid"] = WiFi.SSID();
    doc["wifi"]["rssi"] = WiFi.RSSI();
  }
  doc["bluetooth"] = SerialBT.hasClient();
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleAPICommand() {
  if (server.hasArg("plain")) {
    String command = server.arg("plain");
    
    useSerial = false;
    useBluetooth = false;
    useWebSocket = true;
    
    String response = processCommandWeb(command);
    server.send(200, "text/plain", response);
  } else {
    server.send(400, "text/plain", "No command provided");
  }
}

void handleWiFiScan() {
  int n = WiFi.scanNetworks();
  DynamicJsonDocument doc(2048);
  JsonArray networks = doc.createNestedArray("networks");
  
  for (int i = 0; i < n; i++) {
    JsonObject network = networks.createNestedObject();
    network["ssid"] = WiFi.SSID(i);
    network["rssi"] = WiFi.RSSI(i);
    network["encryption"] = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "open" : "encrypted";
  }
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleWiFiConnect() {
  if (server.hasArg("ssid") && server.hasArg("password")) {
    String ssid = server.arg("ssid");
    String password = server.arg("password");
    
    WiFi.begin(ssid.c_str(), password.c_str());
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      attempts++;
    }
    
    DynamicJsonDocument doc(256);
    if (WiFi.status() == WL_CONNECTED) {
      doc["success"] = true;
      doc["ip"] = WiFi.localIP().toString();
      connectedSSID = ssid;
      wifiConnected = true;
    } else {
      doc["success"] = false;
      doc["error"] = "Connection failed";
    }
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  } else {
    server.send(400, "text/plain", "SSID and password required");
  }
}

void handleGPIORead() {
  DynamicJsonDocument doc(512);
  doc["gpio12"] = digitalRead(USER_GPIO_12);
  doc["gpio13"] = digitalRead(USER_GPIO_13);
  doc["gpio14"] = digitalRead(USER_GPIO_14);
  doc["gpio15"] = digitalRead(USER_GPIO_15);
  doc["gpio16"] = digitalRead(USER_GPIO_16);
  doc["led_builtin"] = digitalRead(LED_BUILTIN);
  doc["led_status"] = digitalRead(LED_STATUS);
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleGPIOWrite() {
  if (server.hasArg("pin") && server.hasArg("value")) {
    int pin = server.arg("pin").toInt();
    int value = server.arg("value").toInt();
    
    if (pin == LED_BUILTIN || pin == LED_STATUS || 
        pin == USER_GPIO_14 || pin == USER_GPIO_15 || pin == USER_GPIO_16) {
      digitalWrite(pin, value);
      server.send(200, "text/plain", "OK");
    } else {
      server.send(400, "text/plain", "Invalid pin");
    }
  } else {
    server.send(400, "text/plain", "Pin and value required");
  }
}

String getMainPage() {
  return R"HTML(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32-CAM Interactive Console</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background: #f0f0f0; }
        .container { max-width: 1200px; margin: 0 auto; }
        .card { background: white; padding: 20px; margin: 10px 0; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        .grid { display: grid; grid-template-columns: 1fr 1fr; gap: 20px; }
        button { background: #007bff; color: white; border: none; padding: 10px 20px; border-radius: 4px; cursor: pointer; }
        button:hover { background: #0056b3; }
        input, select { padding: 8px; border: 1px solid #ddd; border-radius: 4px; margin: 5px; }
        #stream { max-width: 100%; border-radius: 8px; }
        #terminal { background: #000; color: #0f0; font-family: monospace; padding: 10px; height: 300px; overflow-y: scroll; border-radius: 4px; }
        .status { display: inline-block; width: 12px; height: 12px; border-radius: 50%; margin-right: 8px; }
        .online { background: #28a745; }
        .offline { background: #dc3545; }
        .gpio-controls { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 10px; }
        .gpio-item { display: flex; justify-content: space-between; align-items: center; padding: 5px; }
    </style>
</head>
<body>
    <div class="container">
        <h1>ESP32-CAM Interactive Console</h1>
        
        <div class="grid">
            <div class="card">
                <h3>Camera Stream</h3>
                <img id="stream" src="/stream" alt="Camera Stream">
                <br><br>
                <button onclick="capturePhoto()">Capture Photo</button>
                <button onclick="toggleStream()">Toggle Stream</button>
            </div>
            
            <div class="card">
                <h3>System Status</h3>
                <p><span class="status online"></span>Camera: <span id="camera-status">Online</span></p>
                <p><span class="status online"></span>WiFi: <span id="wifi-status">AP Mode</span></p>
                <p><span class="status" id="bt-indicator"></span>Bluetooth: <span id="bt-status">Unknown</span></p>
                <p>Uptime: <span id="uptime">0</span>s</p>
                <p>Free Heap: <span id="heap">0</span> bytes</p>
                <button onclick="updateStatus()">Refresh Status</button>
            </div>
        </div>
        
        <div class="card">
            <h3>Command Terminal</h3>
            <div id="terminal"></div>
            <input type="text" id="command" placeholder="Enter command..." style="width: 80%;">
            <button onclick="sendCommand()">Send</button>
            <button onclick="clearTerminal()">Clear</button>
        </div>
        
        <div class="grid">
            <div class="card">
                <h3>GPIO Control</h3>
                <div class="gpio-controls">
                    <div class="gpio-item">
                        <span>LED Built-in (4):</span>
                        <button onclick="toggleGPIO(4)">Toggle</button>
                    </div>
                    <div class="gpio-item">
                        <span>Status LED (33):</span>
                        <button onclick="toggleGPIO(33)">Toggle</button>
                    </div>
                    <div class="gpio-item">
                        <span>GPIO 14:</span>
                        <button onclick="toggleGPIO(14)">Toggle</button>
                    </div>
                    <div class="gpio-item">
                        <span>GPIO 15:</span>
                        <button onclick="toggleGPIO(15)">Toggle</button>
                    </div>
                    <div class="gpio-item">
                        <span>GPIO 16:</span>
                        <button onclick="toggleGPIO(16)">Toggle</button>
                    </div>
                </div>
            </div>
            
            <div class="card">
                <h3>WiFi Connection</h3>
                <button onclick="scanWiFi()">Scan Networks</button>
                <select id="wifiNetworks"></select>
                <input type="password" id="wifiPassword" placeholder="Password">
                <button onclick="connectWiFi()">Connect</button>
            </div>
        </div>
    </div>

    <script>
        let streamEnabled = true;
        let gpioStates = {};
        
        // Update status periodically
        setInterval(updateStatus, 5000);
        updateStatus();
        
        // Command input handling
        document.getElementById('command').addEventListener('keypress', function(e) {
            if (e.key === 'Enter') {
                sendCommand();
            }
        });
        
        function updateStatus() {
            fetch('/api/status')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('uptime').textContent = Math.floor(data.uptime / 1000);
                    document.getElementById('heap').textContent = data.freeHeap.toLocaleString();
                    document.getElementById('camera-status').textContent = data.camera ? 'Online' : 'Offline';
                    document.getElementById('wifi-status').textContent = data.wifi.connected ? 
                        'Connected to ' + data.wifi.ssid + ' (' + data.wifi.sta_ip + ')' : 'AP Mode Only';
                    document.getElementById('bt-status').textContent = data.bluetooth ? 'Connected' : 'Disconnected';
                    document.getElementById('bt-indicator').className = 'status ' + (data.bluetooth ? 'online' : 'offline');
                });
        }
        
        function sendCommand() {
            const command = document.getElementById('command').value;
            if (!command) return;
            
            document.getElementById('command').value = "";
            appendToTerminal('> ' + command);
            
            fetch('/api/command', {
                method: 'POST',
                body: command
            })
            .then(response => response.text())
            .then(data => {
                appendToTerminal(data);
            });
        }
        
        function appendToTerminal(text) {
            const terminal = document.getElementById('terminal');
            terminal.innerHTML += text + '\n';
            terminal.scrollTop = terminal.scrollHeight;
        }
        
        function clearTerminal() {
            document.getElementById('terminal').innerHTML = "";
        }
        
        function capturePhoto() {
            window.open('/capture', '_blank');
        }
        
        function toggleStream() {
            const img = document.getElementById('stream');
            if (streamEnabled) {
                img.style.display = 'none';
                streamEnabled = false;
            } else {
                img.style.display = 'block';
                img.src = '/stream?' + new Date().getTime();
                streamEnabled = true;
            }
        }
        
        function toggleGPIO(pin) {
            const currentState = gpioStates[pin] || 0;
            const newState = currentState ? 0 : 1;
            
            fetch('/api/gpio/write', {
                method: 'POST',
                headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                body: 'pin=' + pin + '&value=' + newState
            })
            .then(() => {
                gpioStates[pin] = newState;
            });
        }
        
        function scanWiFi() {
            fetch('/api/wifi/scan')
                .then(response => response.json())
                .then(data => {
                    const select = document.getElementById('wifiNetworks');
                    select.innerHTML = "";
                    data.networks.forEach(network => {
                        const option = document.createElement('option');
                        option.value = network.ssid;
                        option.textContent = network.ssid + ' (' + network.rssi + 'dBm)';
                        select.appendChild(option);
                    });
                });
        }
        
        function connectWiFi() {
            const ssid = document.getElementById('wifiNetworks').value;
            const password = document.getElementById('wifiPassword').value;
            
            if (!ssid) {
                alert('Please select a network first');
                return;
            }
            
            fetch('/api/wifi/connect', {
                method: 'POST',
                headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                body: 'ssid=' + encodeURIComponent(ssid) + '&password=' + encodeURIComponent(password)
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    alert('Connected to ' + ssid + '! IP: ' + data.ip);
                    updateStatus();
                } else {
                    alert('Connection failed: ' + data.error);
                }
            });
        }
    </script>
</body>
</html>
)HTML";
}

void printToOutput(const String& message) {
  if (useSerial) {
    Serial.print(message);
  }
  if (useBluetooth) {
    SerialBT.print(message);
  }
}

void printToOutput(const __FlashStringHelper* message) {
  if (useSerial) {
    Serial.print(message);
  }
  if (useBluetooth) {
    SerialBT.print(message);
  }
}

void printlnToOutput(const String& message) {
  if (useSerial) {
    Serial.println(message);
  }
  if (useBluetooth) {
    SerialBT.println(message);
  }
}

void printlnToOutput(const __FlashStringHelper* message) {
  if (useSerial) {
    Serial.println(message);
  }
  if (useBluetooth) {
    SerialBT.println(message);
  }
}

void printWelcomeMessage() {
  Serial.println(F("====================================="));
  Serial.println(F("ESP32-CAM Interactive Console v3.0"));
  Serial.println(F("====================================="));
  Serial.println(F("Features: WiFi, Camera, Bluetooth, Web Interface"));
  Serial.printf("WiFi AP: %s (Password: %s)\n", ap_ssid, ap_password);
  Serial.printf("Web Interface: http://%s\n", WiFi.softAPIP().toString().c_str());
  Serial.printf("mDNS: http://%s.local\n", hostname);
  Serial.println(F("Bluetooth: ESP32-CAM-Console"));
  Serial.println(F("Type 'help' for commands"));
  Serial.println(F("====================================="));
  Serial.print(F("ESP32> "));
  
  SerialBT.println(F("====================================="));
  SerialBT.println(F("ESP32-CAM Interactive Console v3.0"));
  SerialBT.println(F("====================================="));
  SerialBT.println(F("Bluetooth Connected Successfully!"));
  SerialBT.println(F("Type 'help' for commands"));
  SerialBT.println(F("====================================="));
  SerialBT.print(F("ESP32> "));
}

void printPrompt() {
  printToOutput(F("ESP32> "));
}

void processCommand(String command) {
  command.trim();
  command.toLowerCase();
  
  if (command == "help") {
    showHelp();
  }
  else if (command == "status") {
    showStatus();
  }
  else if (command == "info") {
    showSystemInfo();
  }
  else if (command == "memory") {
    showMemoryInfo();
  }
  else if (command == "uptime") {
    showUptime();
  }
  else if (command == "wifi") {
    showWiFiInfo();
  }
  else if (command == "camera") {
    showCameraInfo();
  }
  else if (command == "led on") {
    digitalWrite(LED_BUILTIN, HIGH);
    printlnToOutput(F("LED turned ON"));
  }
  else if (command == "led off") {
    digitalWrite(LED_BUILTIN, LOW);
    printlnToOutput(F("LED turned OFF"));
  }
  else if (command == "blink") {
    blinkLED();
  }
  else if (command == "gpio") {
    showGPIOStatus();
  }
  else if (command.startsWith("gpio ")) {
    handleGPIOCommand(command);
  }
  else if (command == "scan") {
    scanI2C();
  }
  else if (command == "capture") {
    capturePhoto();
  }
  else if (command.startsWith("eeprom ")) {
    handleEEPROM(command);
  }
  else if (command == "restart") {
    printlnToOutput(F("Restarting ESP32..."));
    delay(1000);
    ESP.restart();
  }
  else if (command == "") {
    // Empty command
  }
  else {
    printlnToOutput("Unknown command: " + command);
    printlnToOutput(F("Type 'help' for available commands"));
  }
}

String processCommandWeb(String command) {
  command.trim();
  command.toLowerCase();
  String response = "";
  
  if (command == "help") {
    response = "Available commands:\n";
    response += "Basic: help, status, info, memory, uptime, restart\n";
    response += "WiFi: wifi\n";
    response += "Camera: camera, capture\n";
    response += "LED: led on/off, blink\n";
    response += "GPIO: gpio, gpio <pin> <value>\n";
    response += "Storage: eeprom read/write <addr> [val]\n";
    response += "Tools: scan\n";
  }
  else if (command == "status") {
    response = "=== ESP32-CAM Status ===\n";
    response += "Uptime: " + String(millis() / 1000) + " seconds\n";
    response += "Free Heap: " + String(ESP.getFreeHeap()) + " bytes\n";
    response += "WiFi: ";
    response += (WiFi.isConnected() ? "Connected" : "AP Mode");
    response += "\n";
    response += "Camera: " + String(cameraInitialized ? "Ready" : "Error") + "\n";
    response += "Bluetooth: " + String(SerialBT.hasClient() ? "Connected" : "Disconnected") + "\n";
  }
  else {
    // Process other commands and capture output
    processCommand(command);
    response = "Command executed: " + command;
  }
  
  return response;
}

void showHelp() {
  printlnToOutput(F("=== ESP32-CAM Interactive Console Commands ==="));
  printlnToOutput(F("Basic: help, status, info, memory, uptime, restart"));
  printlnToOutput(F("WiFi: wifi"));
  printlnToOutput(F("Camera: camera, capture"));
  printlnToOutput(F("LED: led on/off, blink"));
  printlnToOutput(F("GPIO: gpio, gpio <pin> <value>"));
  printlnToOutput(F("Storage: eeprom read/write <addr> [val]"));
  printlnToOutput(F("Tools: scan"));
  printToOutput(F("Web Interface: http://"));
  printlnToOutput(WiFi.softAPIP().toString());
}

void showStatus() {
  printlnToOutput(F("=== ESP32-CAM Status ==="));
  printToOutput(F("Uptime: "));
  printToOutput(String(millis() / 1000));
  printlnToOutput(F(" seconds"));
  
  printToOutput(F("Free Heap: "));
  printToOutput(String(ESP.getFreeHeap()));
  printlnToOutput(F(" bytes"));
  
  printToOutput(F("Chip: "));
  printToOutput(ESP.getChipModel());
  printToOutput(F(" Rev "));
  printlnToOutput(String(ESP.getChipRevision()));
  
  printToOutput(F("Flash: "));
  printToOutput(String(ESP.getFlashChipSize() / 1024 / 1024));
  printlnToOutput(F(" MB"));
  
  printToOutput(F("WiFi: "));
  if (WiFi.isConnected()) {
    printToOutput(F("Connected to "));
    printToOutput(WiFi.SSID());
    printToOutput(F(" ("));
    printToOutput(WiFi.localIP().toString());
    printlnToOutput(F(")"));
  } else {
    printlnToOutput(F("AP Mode"));
  }
  
  printToOutput(F("Camera: "));
  printlnToOutput(cameraInitialized ? F("Ready") : F("Error"));
  
  printToOutput(F("Bluetooth: "));
  printlnToOutput(SerialBT.hasClient() ? F("Connected") : F("Disconnected"));
}

void showSystemInfo() {
  printlnToOutput(F("=== System Information ==="));
  printToOutput(F("Chip: "));
  printToOutput(ESP.getChipModel());
  printToOutput(F(" Rev "));
  printlnToOutput(String(ESP.getChipRevision()));
  
  printToOutput(F("CPU Frequency: "));
  printToOutput(String(ESP.getCpuFreqMHz()));
  printlnToOutput(F(" MHz"));
  
  printToOutput(F("Flash Size: "));
  printToOutput(String(ESP.getFlashChipSize() / 1024 / 1024));
  printlnToOutput(F(" MB"));
  
  printToOutput(F("PSRAM: "));
  printlnToOutput(psramFound() ? F("Available") : F("Not found"));
  
  printToOutput(F("SDK Version: "));
  printlnToOutput(ESP.getSdkVersion());
  
  printToOutput(F("MAC Address: "));
  printlnToOutput(WiFi.macAddress());
}

void showMemoryInfo() {
  printlnToOutput(F("=== Memory Information ==="));
  printToOutput(F("Free Heap: "));
  printToOutput(String(ESP.getFreeHeap()));
  printlnToOutput(F(" bytes"));
  
  printToOutput(F("Largest Free Block: "));
  printToOutput(String(ESP.getMaxAllocHeap()));
  printlnToOutput(F(" bytes"));
  
  printToOutput(F("Total Heap: "));
  printToOutput(String(ESP.getHeapSize()));
  printlnToOutput(F(" bytes"));
  
  if (psramFound()) {
    printToOutput(F("Free PSRAM: "));
    printToOutput(String(ESP.getFreePsram()));
    printlnToOutput(F(" bytes"));
    
    printToOutput(F("Total PSRAM: "));
    printToOutput(String(ESP.getPsramSize()));
    printlnToOutput(F(" bytes"));
  }
}

void showUptime() {
  unsigned long ms = millis();
  unsigned long seconds = ms / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  unsigned long days = hours / 24;
  
  printlnToOutput(F("=== Uptime Information ==="));
  printToOutput(F("Total milliseconds: "));
  printlnToOutput(String(ms));
  
  printToOutput(F("Uptime: "));
  if (days > 0) {
    printToOutput(String(days));
    printToOutput(F(" days, "));
  }
  if (hours % 24 > 0 || days > 0) {
    printToOutput(String(hours % 24));
    printToOutput(F(" hours, "));
  }
  if (minutes % 60 > 0 || hours > 0) {
    printToOutput(String(minutes % 60));
    printToOutput(F(" minutes, "));
  }
  printToOutput(String(seconds % 60));
  printlnToOutput(F(" seconds"));
}

void showWiFiInfo() {
  printlnToOutput(F("=== WiFi Information ==="));
  printToOutput(F("Mode: "));
  printlnToOutput(String(WiFi.getMode()));
  
  printToOutput(F("AP SSID: "));
  printlnToOutput(ap_ssid);
  printToOutput(F("AP IP: "));
  printlnToOutput(WiFi.softAPIP().toString());
  printToOutput(F("AP Clients: "));
  printlnToOutput(String(WiFi.softAPgetStationNum()));
  
  if (WiFi.isConnected()) {
    printToOutput(F("Connected to: "));
    printlnToOutput(WiFi.SSID());
    printToOutput(F("IP Address: "));
    printlnToOutput(WiFi.localIP().toString());
    printToOutput(F("Signal Strength: "));
    printToOutput(String(WiFi.RSSI()));
    printlnToOutput(F(" dBm"));
  } else {
    printlnToOutput(F("Not connected to any network"));
  }
  
  printToOutput(F("MAC Address: "));
  printlnToOutput(WiFi.macAddress());
}

void showCameraInfo() {
  printlnToOutput(F("=== Camera Information ==="));
  printToOutput(F("Status: "));
  printlnToOutput(cameraInitialized ? F("Initialized") : F("Error"));
  
  if (cameraInitialized) {
    sensor_t* s = esp_camera_sensor_get();
    printToOutput(F("Sensor: "));
    printlnToOutput(F("OV2640"));
    
    camera_fb_t* fb = esp_camera_fb_get();
    if (fb) {
      printToOutput(F("Frame Size: "));
      printToOutput(String(fb->width));
      printToOutput(F("x"));
      printlnToOutput(String(fb->height));
      
      printToOutput(F("Image Size: "));
      printToOutput(String(fb->len));
      printlnToOutput(F(" bytes"));
      
      esp_camera_fb_return(fb);
    }
    
    printToOutput(F("Stream: http://"));
    printToOutput(WiFi.softAPIP().toString());
    printlnToOutput(F("/stream"));
    printToOutput(F("Capture: http://"));
    printToOutput(WiFi.softAPIP().toString());
    printlnToOutput(F("/capture"));
  }
}

void blinkLED() {
  printlnToOutput(F("Blinking LED 5 times..."));
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
    printToOutput(F("."));
  }
  printlnToOutput(F(" Done!"));
}

void showGPIOStatus() {
  printlnToOutput(F("=== GPIO Status ==="));
  printToOutput(F("GPIO 12 (Input): "));
  printlnToOutput(digitalRead(USER_GPIO_12) ? F("HIGH") : F("LOW"));
  
  printToOutput(F("GPIO 13 (Input): "));
  printlnToOutput(digitalRead(USER_GPIO_13) ? F("HIGH") : F("LOW"));
  
  printToOutput(F("GPIO 14 (Output): "));
  printlnToOutput(digitalRead(USER_GPIO_14) ? F("HIGH") : F("LOW"));
  
  printToOutput(F("GPIO 15 (Output): "));
  printlnToOutput(digitalRead(USER_GPIO_15) ? F("HIGH") : F("LOW"));
  
  printToOutput(F("GPIO 16 (Output): "));
  printlnToOutput(digitalRead(USER_GPIO_16) ? F("HIGH") : F("LOW"));
  
  printToOutput(F("LED Built-in (4): "));
  printlnToOutput(digitalRead(LED_BUILTIN) ? F("ON") : F("OFF"));
  
  printToOutput(F("LED Status (33): "));
  printlnToOutput(digitalRead(LED_STATUS) ? F("ON") : F("OFF"));
}

void handleGPIOCommand(String command) {
  int firstSpace = command.indexOf(' ');
  int secondSpace = command.indexOf(' ', firstSpace + 1);
  
  if (firstSpace == -1 || secondSpace == -1) {
    printlnToOutput(F("Usage: gpio <pin> <value>"));
    printlnToOutput(F("Available pins: 4, 14, 15, 16, 33"));
    return;
  }
  
  int pin = command.substring(firstSpace + 1, secondSpace).toInt();
  int value = command.substring(secondSpace + 1).toInt();
  
  if (pin == LED_BUILTIN || pin == LED_STATUS || 
      pin == USER_GPIO_14 || pin == USER_GPIO_15 || pin == USER_GPIO_16) {
    digitalWrite(pin, value);
    printToOutput(F("GPIO "));
    printToOutput(String(pin));
    printToOutput(F(" set to "));
    printlnToOutput(value ? F("HIGH") : F("LOW"));
  } else {
    printlnToOutput(F("Error: Invalid pin number"));
    printlnToOutput(F("Available pins: 4, 14, 15, 16, 33"));
  }
}

void scanI2C() {
  printlnToOutput(F("=== I2C Device Scanner ==="));
  
  byte error, address;
  int devices = 0;
  
  printlnToOutput(F("Scanning I2C bus (0x03-0x77)..."));
  
  for (address = 3; address < 120; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      printToOutput(F("I2C device found at 0x"));
      if (address < 16) printToOutput(F("0"));
      printlnToOutput(String(address, HEX));
      devices++;
    }
  }
  
  if (devices == 0) {
    printlnToOutput(F("No I2C devices found"));
  } else {
    printToOutput(F("Found "));
    printToOutput(String(devices));
    printlnToOutput(F(" device(s)"));
  }
}

void capturePhoto() {
  if (!cameraInitialized) {
    printlnToOutput(F("Error: Camera not initialized"));
    return;
  }
  
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    printlnToOutput(F("Error: Camera capture failed"));
    return;
  }
  
  printlnToOutput(F("Photo captured successfully!"));
  printToOutput(F("Image size: "));
  printToOutput(String(fb->len));
  printlnToOutput(F(" bytes"));
  printToOutput(F("Resolution: "));
  printToOutput(String(fb->width));
  printToOutput(F("x"));
  printlnToOutput(String(fb->height));
  printToOutput(F("Download: http://"));
  printToOutput(WiFi.softAPIP().toString());
  printlnToOutput(F("/capture"));
  
  esp_camera_fb_return(fb);
}

void handleEEPROM(String command) {
  int firstSpace = command.indexOf(' ');
  if (firstSpace == -1) {
    printlnToOutput(F("Usage: eeprom read <addr> or eeprom write <addr> <value>"));
    return;
  }
  
  String operation = command.substring(firstSpace + 1);
  
  if (operation.startsWith("read ")) {
    int addr = operation.substring(5).toInt();
    if (addr < 0 || addr >= 512) {
      printlnToOutput(F("Error: Address must be 0-511"));
      return;
    }
    
    byte value = EEPROM.read(addr);
    printToOutput(F("EEPROM["));
    printToOutput(String(addr));
    printToOutput(F("] = "));
    printToOutput(String(value));
    printToOutput(F(" (0x"));
    printToOutput(String(value, HEX));
    printlnToOutput(F(")"));
  }
  else if (operation.startsWith("write ")) {
    int secondSpace = operation.indexOf(' ', 6);
    if (secondSpace == -1) {
      printlnToOutput(F("Usage: eeprom write <addr> <value>"));
      return;
    }
    
    int addr = operation.substring(6, secondSpace).toInt();
    int value = operation.substring(secondSpace + 1).toInt();
    
    if (addr < 0 || addr >= 512) {
      printlnToOutput(F("Error: Address must be 0-511"));
      return;
    }
    
    if (value < 0 || value > 255) {
      printlnToOutput(F("Error: Value must be 0-255"));
      return;
    }
    
    EEPROM.write(addr, value);
    EEPROM.commit();
    printToOutput(F("EEPROM["));
    printToOutput(String(addr));
    printToOutput(F("] = "));
    printlnToOutput(String(value));
  }
  else {
    printlnToOutput(F("Usage: eeprom read <addr> or eeprom write <addr> <value>"));
  }
}
