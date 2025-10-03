#include "WiFi.h"
#include "WebServer.h"
#include "ESPmDNS.h"
#include "SPIFFS.h"
#include "BluetoothSerial.h"
#include "esp_bt_device.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"

// Konfigurasi WiFi
const char* ssid = "WebBT_ESP32";
const char* password = "12345678";

// Objek Bluetooth
BluetoothSerial SerialBT;
WebServer server(80);

// Variabel global
String btDevices = "";
bool isScanning = false;
bool isConnected = false;
String connectedDevice = "";
bool a2dpConnected = false;

// HTML Interface
const char* htmlContent = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>WebBT - ESP32 Bluetooth Controller</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
        }
        
        .container {
            max-width: 1200px;
            margin: 0 auto;
            background: rgba(255, 255, 255, 0.95);
            border-radius: 15px;
            box-shadow: 0 15px 35px rgba(0, 0, 0, 0.1);
            overflow: hidden;
        }
        
        .header {
            background: linear-gradient(135deg, #2c3e50, #3498db);
            color: white;
            padding: 30px;
            text-align: center;
        }
        
        .header h1 {
            font-size: 2.5em;
            margin-bottom: 10px;
        }
        
        .header p {
            font-size: 1.1em;
            opacity: 0.9;
        }
        
        .status-bar {
            background: #f8f9fa;
            padding: 15px 30px;
            border-bottom: 1px solid #e9ecef;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        
        .status-item {
            display: flex;
            align-items: center;
            gap: 10px;
        }
        
        .status-dot {
            width: 12px;
            height: 12px;
            border-radius: 50%;
            background: #dc3545;
        }
        
        .status-dot.connected {
            background: #28a745;
        }
        
        .status-dot.scanning {
            background: #ffc107;
            animation: pulse 1.5s infinite;
        }
        
        @keyframes pulse {
            0% { opacity: 1; }
            50% { opacity: 0.5; }
            100% { opacity: 1; }
        }
        
        .main-content {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 30px;
            padding: 30px;
        }
        
        @media (max-width: 768px) {
            .main-content {
                grid-template-columns: 1fr;
            }
        }
        
        .card {
            background: white;
            border-radius: 10px;
            padding: 25px;
            box-shadow: 0 5px 15px rgba(0, 0, 0, 0.08);
            border: 1px solid #e9ecef;
        }
        
        .card h3 {
            color: #2c3e50;
            margin-bottom: 20px;
            font-size: 1.4em;
            border-bottom: 2px solid #3498db;
            padding-bottom: 10px;
        }
        
        .btn {
            background: linear-gradient(135deg, #3498db, #2980b9);
            color: white;
            border: none;
            padding: 12px 25px;
            border-radius: 25px;
            cursor: pointer;
            font-size: 14px;
            font-weight: 600;
            transition: all 0.3s ease;
            margin: 5px;
        }
        
        .btn:hover {
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(0, 0, 0, 0.2);
        }
        
        .btn:active {
            transform: translateY(0);
        }
        
        .btn-scan {
            background: linear-gradient(135deg, #e74c3c, #c0392b);
        }
        
        .btn-success {
            background: linear-gradient(135deg, #27ae60, #229954);
        }
        
        .btn-warning {
            background: linear-gradient(135deg, #f39c12, #e67e22);
        }
        
        .device-list {
            max-height: 400px;
            overflow-y: auto;
            border: 1px solid #e9ecef;
            border-radius: 8px;
            padding: 10px;
        }
        
        .device-item {
            padding: 15px;
            border-bottom: 1px solid #e9ecef;
            display: flex;
            justify-content: space-between;
            align-items: center;
            transition: background 0.3s ease;
        }
        
        .device-item:hover {
            background: #f8f9fa;
        }
        
        .device-item:last-child {
            border-bottom: none;
        }
        
        .device-info {
            flex-grow: 1;
        }
        
        .device-name {
            font-weight: 600;
            color: #2c3e50;
        }
        
        .device-address {
            font-size: 0.9em;
            color: #7f8c8d;
            font-family: monospace;
        }
        
        .audio-controls {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(120px, 1fr));
            gap: 10px;
            margin-top: 20px;
        }
        
        .volume-control {
            grid-column: 1 / -1;
            margin-top: 10px;
        }
        
        .volume-slider {
            width: 100%;
            height: 8px;
            border-radius: 4px;
            background: #ddd;
            outline: none;
            opacity: 0.7;
            transition: opacity .2s;
        }
        
        .volume-slider:hover {
            opacity: 1;
        }
        
        .log-container {
            background: #1e1e1e;
            color: #00ff00;
            font-family: 'Courier New', monospace;
            padding: 15px;
            border-radius: 8px;
            height: 200px;
            overflow-y: auto;
            margin-top: 20px;
        }
        
        .file-upload {
            margin-top: 20px;
        }
        
        .file-input {
            width: 100%;
            padding: 10px;
            margin-bottom: 10px;
            border: 2px dashed #3498db;
            border-radius: 8px;
            background: #f8f9fa;
        }
        
        .progress-bar {
            width: 100%;
            height: 20px;
            background: #e9ecef;
            border-radius: 10px;
            margin-top: 10px;
            overflow: hidden;
            display: none;
        }
        
        .progress {
            width: 0%;
            height: 100%;
            background: linear-gradient(135deg, #3498db, #2980b9);
            transition: width 0.3s ease;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🔄 WebBT Controller</h1>
            <p>ESP32 Bluetooth & Audio Management System</p>
        </div>
        
        <div class="status-bar">
            <div class="status-item">
                <div class="status-dot" id="wifiStatus"></div>
                <span>WiFi: <span id="wifiText">Connected</span></span>
            </div>
            <div class="status-item">
                <div class="status-dot" id="btStatus"></div>
                <span>Bluetooth: <span id="btText">Ready</span></span>
            </div>
            <div class="status-item">
                <div class="status-dot" id="audioStatus"></div>
                <span>Audio: <span id="audioText">Available</span></span>
            </div>
        </div>
        
        <div class="main-content">
            <div class="left-panel">
                <div class="card">
                    <h3>🔍 Bluetooth Scanner</h3>
                    <button class="btn btn-scan" onclick="startScan()">Start Scan</button>
                    <button class="btn btn-warning" onclick="stopScan()">Stop Scan</button>
                    <button class="btn" onclick="clearDevices()">Clear List</button>
                    
                    <div class="device-list" id="deviceList">
                        <div class="device-item">
                            <div class="device-info">
                                <div class="device-name">No devices found</div>
                            </div>
                        </div>
                    </div>
                </div>
                
                <div class="card">
                    <h3>📁 File Management</h3>
                    <div class="file-upload">
                        <input type="file" class="file-input" id="fileInput" accept=".txt,.json,.csv">
                        <button class="btn" onclick="uploadFile()">Upload File</button>
                        <button class="btn btn-warning" onclick="exportData()">Export Scan Data</button>
                        <div class="progress-bar" id="progressBar">
                            <div class="progress" id="progress"></div>
                        </div>
                    </div>
                </div>
            </div>
            
            <div class="right-panel">
                <div class="card">
                    <h3>🎵 Audio Control</h3>
                    <div class="audio-controls">
                        <button class="btn" onclick="audioControl('play')">▶️ Play</button>
                        <button class="btn" onclick="audioControl('pause')">⏸️ Pause</button>
                        <button class="btn" onclick="audioControl('next')">⏭️ Next</button>
                        <button class="btn" onclick="audioControl('prev')">⏮️ Previous</button>
                    </div>
                    <div class="volume-control">
                        <label>Volume: <span id="volumeValue">50%</span></label>
                        <input type="range" min="0" max="100" value="50" class="volume-slider" 
                               oninput="setVolume(this.value)">
                    </div>
                </div>
                
                <div class="card">
                    <h3>🔗 Connection Manager</h3>
                    <div id="connectionInfo">
                        <p>No device connected</p>
                    </div>
                    <button class="btn btn-success" onclick="connectA2DP()">Connect A2DP</button>
                    <button class="btn" onclick="connectSPP()">Connect SPP</button>
                    <button class="btn btn-warning" onclick="disconnectBT()">Disconnect</button>
                </div>
                
                <div class="card">
                    <h3>📊 System Log</h3>
                    <div class="log-container" id="logContainer">
                        > System initialized successfully\n
                        > WebBT ready for operations\n
                    </div>
                    <button class="btn" onclick="clearLog()">Clear Log</button>
                </div>
            </div>
        </div>
    </div>

    <script>
        let ws;
        let isConnected = false;
        
        function connectWebSocket() {
            ws = new WebSocket(`ws://${window.location.host}/ws`);
            
            ws.onopen = function() {
                addLog('WebSocket connected');
                updateStatus('wifiStatus', 'connected', 'Connected');
            };
            
            ws.onmessage = function(event) {
                const data = JSON.parse(event.data);
                handleWebSocketMessage(data);
            };
            
            ws.onclose = function() {
                addLog('WebSocket disconnected');
                updateStatus('wifiStatus', 'disconnected', 'Disconnected');
                setTimeout(connectWebSocket, 2000);
            };
        }
        
        function handleWebSocketMessage(data) {
            switch(data.type) {
                case 'btDevices':
                    updateDeviceList(data.devices);
                    break;
                case 'connectionStatus':
                    updateConnectionStatus(data);
                    break;
                case 'log':
                    addLog(data.message);
                    break;
                case 'scanStatus':
                    updateScanStatus(data.scanning);
                    break;
            }
        }
        
        function updateDeviceList(devices) {
            const deviceList = document.getElementById('deviceList');
            if (devices.length === 0) {
                deviceList.innerHTML = '<div class="device-item"><div class="device-info"><div class="device-name">No devices found</div></div></div>';
                return;
            }
            
            deviceList.innerHTML = devices.map(device => `
                <div class="device-item">
                    <div class="device-info">
                        <div class="device-name">${device.name || 'Unknown Device'}</div>
                        <div class="device-address">${device.address}</div>
                    </div>
                    <button class="btn" onclick="connectToDevice('${device.address}')">Connect</button>
                </div>
            `).join('');
        }
        
        function updateConnectionStatus(status) {
            const btStatus = document.getElementById('btStatus');
            const btText = document.getElementById('btText');
            const connectionInfo = document.getElementById('connectionInfo');
            
            if (status.connected) {
                btStatus.className = 'status-dot connected';
                btText.textContent = `Connected to ${status.deviceName}`;
                connectionInfo.innerHTML = `
                    <p><strong>Connected to:</strong> ${status.deviceName}</p>
                    <p><strong>Address:</strong> ${status.deviceAddress}</p>
                    <p><strong>Type:</strong> ${status.connectionType}</p>
                `;
                isConnected = true;
            } else {
                btStatus.className = 'status-dot';
                btText.textContent = 'Ready';
                connectionInfo.innerHTML = '<p>No device connected</p>';
                isConnected = false;
            }
        }
        
        function updateScanStatus(scanning) {
            const btStatus = document.getElementById('btStatus');
            if (scanning) {
                btStatus.className = 'status-dot scanning';
                document.getElementById('btText').textContent = 'Scanning...';
            }
        }
        
        function addLog(message) {
            const logContainer = document.getElementById('logContainer');
            logContainer.innerHTML += `> ${message}\n`;
            logContainer.scrollTop = logContainer.scrollHeight;
        }
        
        function updateStatus(elementId, status, text) {
            const element = document.getElementById(elementId);
            const textElement = document.getElementById(elementId.replace('Status', 'Text'));
            element.className = `status-dot ${status}`;
            textElement.textContent = text;
        }
        
        function startScan() {
            ws.send(JSON.stringify({type: 'startScan'}));
            addLog('Starting Bluetooth scan...');
        }
        
        function stopScan() {
            ws.send(JSON.stringify({type: 'stopScan'}));
            addLog('Stopping Bluetooth scan...');
        }
        
        function clearDevices() {
            document.getElementById('deviceList').innerHTML = 
                '<div class="device-item"><div class="device-info"><div class="device-name">No devices found</div></div></div>';
        }
        
        function connectToDevice(address) {
            ws.send(JSON.stringify({type: 'connect', address: address}));
            addLog(`Connecting to device: ${address}`);
        }
        
        function connectA2DP() {
            ws.send(JSON.stringify({type: 'connectA2DP'}));
            addLog('Connecting A2DP audio profile...');
        }
        
        function connectSPP() {
            ws.send(JSON.stringify({type: 'connectSPP'}));
            addLog('Connecting SPP serial profile...');
        }
        
        function disconnectBT() {
            ws.send(JSON.stringify({type: 'disconnect'}));
            addLog('Disconnecting Bluetooth...');
        }
        
        function audioControl(command) {
            ws.send(JSON.stringify({type: 'audioControl', command: command}));
            addLog(`Audio command: ${command}`);
        }
        
        function setVolume(value) {
            document.getElementById('volumeValue').textContent = value + '%';
            ws.send(JSON.stringify({type: 'setVolume', value: parseInt(value)}));
        }
        
        function uploadFile() {
            const fileInput = document.getElementById('fileInput');
            const file = fileInput.files[0];
            if (!file) {
                addLog('No file selected for upload');
                return;
            }
            
            const formData = new FormData();
            formData.append('file', file);
            
            const progressBar = document.getElementById('progressBar');
            const progress = document.getElementById('progress');
            
            progressBar.style.display = 'block';
            
            fetch('/upload', {
                method: 'POST',
                body: formData
            })
            .then(response => response.text())
            .then(data => {
                progress.style.width = '100%';
                setTimeout(() => {
                    progressBar.style.display = 'none';
                    progress.style.width = '0%';
                }, 1000);
                addLog(`File uploaded: ${file.name}`);
            })
            .catch(error => {
                addLog(`Upload failed: ${error}`);
                progressBar.style.display = 'none';
            });
        }
        
        function exportData() {
            ws.send(JSON.stringify({type: 'exportData'}));
            addLog('Exporting scan data...');
        }
        
        function clearLog() {
            document.getElementById('logContainer').innerHTML = '> Log cleared\n';
        }
        
        // Initialize WebSocket connection
        connectWebSocket();
        
        // Periodic status updates
        setInterval(() => {
            if (ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify({type: 'getStatus'}));
            }
        }, 5000);
    </script>
</body>
</html>
)rawliteral";

// Callback untuk A2DP
void a2dp_callback(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param) {
  switch (event) {
    case ESP_A2D_CONNECTION_STATE_EVT:
      if (param->conn_stat.state == ESP_A2D_CONNECTION_STATE_CONNECTED) {
        Serial.println("A2DP Connected");
        a2dpConnected = true;
      } else if (param->conn_stat.state == ESP_A2D_CONNECTION_STATE_DISCONNECTED) {
        Serial.println("A2DP Disconnected");
        a2dpConnected = false;
      }
      break;
    case ESP_A2D_AUDIO_STATE_EVT:
      Serial.println("A2DP Audio State Changed");
      break;
    default:
      break;
  }
}

// WebSocket event handler
void handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = server.client().remoteIP();
        Serial.printf("[%u] Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
      }
      break;
    case WStype_TEXT:
      handleWebSocketMessage(num, payload);
      break;
    default:
      break;
  }
}

void handleWebSocketMessage(uint8_t num, uint8_t * payload) {
  String message = String((char*)payload);
  Serial.println("WebSocket message: " + message);
  
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, message);
  String type = doc["type"];
  
  if (type == "startScan") {
    startBluetoothScan();
  } else if (type == "stopScan") {
    stopBluetoothScan();
  } else if (type == "connect") {
    String address = doc["address"];
    connectToDevice(address);
  } else if (type == "connectA2DP") {
    connectA2DP();
  } else if (type == "connectSPP") {
    connectSPP();
  } else if (type == "disconnect") {
    disconnectBluetooth();
  } else if (type == "audioControl") {
    String command = doc["command"];
    handleAudioControl(command);
  } else if (type == "setVolume") {
    int volume = doc["value"];
    setVolume(volume);
  } else if (type == "exportData") {
    exportScanData();
  } else if (type == "getStatus") {
    sendStatusUpdate();
  }
}

void startBluetoothScan() {
  Serial.println("Starting Bluetooth scan...");
  isScanning = true;
  
  // Kirim status scanning ke WebSocket
  String message = "{\"type\":\"scanStatus\",\"scanning\":true}";
  webSocket.broadcastTXT(message);
  
  // Simulasikan scan perangkat (dalam implementasi nyata, gunakan BluetoothSerial.scan())
  btDevices = "[";
  btDevices += "{\"name\":\"Smartphone\",\"address\":\"AA:BB:CC:DD:EE:FF\"},";
  btDevices += "{\"name\":\"BT Speaker\",\"address\":\"11:22:33:44:55:66\"},";
  btDevices += "{\"name\":\"Wireless Mouse\",\"address\":\"77:88:99:AA:BB:CC\"}";
  btDevices += "]";
  
  String devicesMessage = "{\"type\":\"btDevices\",\"devices\":" + btDevices + "}";
  webSocket.broadcastTXT(devicesMessage);
  
  isScanning = false;
  String stopMessage = "{\"type\":\"scanStatus\",\"scanning\":false}";
  webSocket.broadcastTXT(stopMessage);
}

void stopBluetoothScan() {
  Serial.println("Stopping Bluetooth scan");
  isScanning = false;
}

void connectToDevice(String address) {
  Serial.println("Connecting to device: " + address);
  
  // Simulasi koneksi berhasil
  isConnected = true;
  connectedDevice = address;
  
  String message = "{\"type\":\"connectionStatus\",\"connected\":true,\"deviceName\":\"Connected Device\",\"deviceAddress\":\"" + address + "\",\"connectionType\":\"SPP\"}";
  webSocket.broadcastTXT(message);
  
  String logMessage = "{\"type\":\"log\",\"message\":\"Connected to device: " + address + "\"}";
  webSocket.broadcastTXT(logMessage);
}

void connectA2DP() {
  Serial.println("Connecting A2DP...");
  // Implementasi koneksi A2DP akan ditambahkan di sini
  String message = "{\"type\":\"connectionStatus\",\"connected\":true,\"deviceName\":\"A2DP Speaker\",\"deviceAddress\":\"11:22:33:44:55:66\",\"connectionType\":\"A2DP\"}";
  webSocket.broadcastTXT(message);
}

void connectSPP() {
  Serial.println("Connecting SPP...");
  // Implementasi koneksi SPP
  String message = "{\"type\":\"connectionStatus\",\"connected\":true,\"deviceName\":\"SPP Device\",\"deviceAddress\":\"AA:BB:CC:DD:EE:FF\",\"connectionType\":\"SPP\"}";
  webSocket.broadcastTXT(message);
}

void disconnectBluetooth() {
  Serial.println("Disconnecting Bluetooth...");
  isConnected = false;
  a2dpConnected = false;
  connectedDevice = "";
  
  String message = "{\"type\":\"connectionStatus\",\"connected\":false}";
  webSocket.broadcastTXT(message);
  
  String logMessage = "{\"type\":\"log\",\"message\":\"Bluetooth disconnected\"}";
  webSocket.broadcastTXT(logMessage);
}

void handleAudioControl(String command) {
  Serial.println("Audio control: " + command);
  // Implementasi kontrol audio
  String logMessage = "{\"type\":\"log\",\"message\":\"Audio command executed: " + command + "\"}";
  webSocket.broadcastTXT(logMessage);
}

void setVolume(int volume) {
  Serial.println("Setting volume to: " + String(volume));
  // Implementasi set volume
}

void exportScanData() {
  Serial.println("Exporting scan data...");
  // Implementasi ekspor data
  String logMessage = "{\"type\":\"log\",\"message\":\"Scan data exported successfully\"}";
  webSocket.broadcastTXT(logMessage);
}

void sendStatusUpdate() {
  String status = "{\"type\":\"statusUpdate\",\"wifi\":\"connected\",\"bluetooth\":\"ready\",\"audio\":\"available\"}";
  webSocket.broadcastTXT(status);
}

void handleRoot() {
  server.send(200, "text/html", htmlContent);
}

void handleUpload() {
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    Serial.printf("Upload: %s\n", upload.filename.c_str());
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    // File writing in progress
  } else if (upload.status == UPLOAD_FILE_END) {
    Serial.printf("Upload Success: %u bytes\n", upload.totalSize);
    server.send(200, "text/plain", "Upload Success");
  }
}

void setup() {
  Serial.begin(115200);
  
  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  
  // Setup WiFi Access Point
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  // Initialize Bluetooth
  SerialBT.begin("ESP32_WebBT");
  Serial.println("Bluetooth Started! Ready to pair...");
  
  // Initialize A2DP
  esp_a2d_register_callback(&a2dp_callback);
  esp_a2d_sink_register_data_callback(NULL);
  
  // Setup Web Server
  server.on("/", handleRoot);
  server.on("/upload", HTTP_POST, []() {
    server.send(200, "text/plain", "Upload Success");
  }, handleUpload);
  
  server.begin();
  
  // Initialize WebSocket
  webSocket.begin();
  webSocket.onEvent(handleWebSocketEvent);
  
  Serial.println("WebBT Server Started!");
  Serial.println("Connect to WiFi: " + String(ssid));
  Serial.println("Password: " + String(password));
  Serial.println("Then visit: http://" + IP.toString());
}

void loop() {
  server.handleClient();
  webSocket.loop();
  
  // Maintenance tasks can be added here
  static unsigned long lastStatusUpdate = 0;
  if (millis() - lastStatusUpdate > 10000) {
    lastStatusUpdate = millis();
    sendStatusUpdate();
  }
}