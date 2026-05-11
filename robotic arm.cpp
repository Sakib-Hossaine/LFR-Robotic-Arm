#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>
#include "esp_camera.h"

// WiFi credentials
const char* ssid = "ESP32-CAM";
const char* password = "12345678";

// Camera pins for ESP32-CAM
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

// Servo pins (adjust according to your connections)
#define SERVO_BASE_PIN     12  // Base rotation
#define SERVO_SHOULDER_PIN 13  // Shoulder joint
#define SERVO_ELBOW_PIN    14  // Elbow joint
#define SERVO_GRIPPER_PIN  15  // Gripper

// Create servo objects
Servo servoBase;
Servo servoShoulder;
Servo servoElbow;
Servo servoGripper;

// Current servo angles
int baseAngle = 90;
int shoulderAngle = 90;
int elbowAngle = 90;
int gripperAngle = 90;  // 90 = open, 0 = closed

// Web server on port 80
WebServer server(80);

// Forward declarations (function prototypes)
void moveToHome();
void updateAllServos();
void handleServoControl();
void handlePreset();
void handleGetPositions();
void setupCamera();
void setupServos();

// HTML page
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32-CAM 4-DOF Robot Arm</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            background: #1a1a1a;
            color: #fff;
            margin: 0;
            padding: 10px;
        }
        h1 {
            color: #00ff88;
            margin: 10px 0;
        }
        .container {
            max-width: 800px;
            margin: 0 auto;
        }
        .video-container {
            margin: 10px 0;
        }
        img {
            max-width: 100%;
            border: 2px solid #00ff88;
            border-radius: 5px;
        }
        .controls {
            background: #2d2d2d;
            padding: 15px;
            border-radius: 10px;
            margin: 10px 0;
        }
        .joint-control {
            background: #3d3d3d;
            padding: 10px;
            margin: 10px 0;
            border-radius: 8px;
        }
        input[type="range"] {
            width: 80%;
            margin: 10px;
        }
        button {
            background: #00ff88;
            color: #1a1a1a;
            border: none;
            padding: 10px 20px;
            margin: 5px;
            border-radius: 5px;
            font-size: 16px;
            cursor: pointer;
        }
        button:hover {
            background: #00cc66;
        }
        .value {
            display: inline-block;
            width: 50px;
            text-align: center;
        }
        .quick-moves button {
            background: #ff8800;
            color: white;
            padding: 8px 12px;
            font-size: 14px;
        }
        .quick-moves button:hover {
            background: #cc6600;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1> ESP32-CAM Robot Arm</h1>
        
        <div class="video-container">
            <img src="/stream" id="stream" crossorigin>
        </div>
        
        <div class="controls">
            <div class="joint-control">
                <h3>Base Rotation</h3>
                <input type="range" id="baseSlider" min="0" max="180" value="90" onchange="updateServo('base', this.value)">
                <span class="value"><span id="baseValue">90</span>degree</span>
            </div>
            
            <div class="joint-control">
                <h3>Shoulder</h3>
                <input type="range" id="shoulderSlider" min="0" max="180" value="90" onchange="updateServo('shoulder', this.value)">
                <span class="value"><span id="shoulderValue">90</span>degree</span>
            </div>
            
            <div class="joint-control">
                <h3>Elbow</h3>
                <input type="range" id="elbowSlider" min="0" max="180" value="90" onchange="updateServo('elbow', this.value)">
                <span class="value"><span id="elbowValue">90</span>degree</span>
            </div>
            
            <div class="joint-control">
                <h3>Gripper</h3>
                <input type="range" id="gripperSlider" min="0" max="90" value="90" onchange="updateServo('gripper', this.value)">
                <span class="value"><span id="gripperValue">90</span>degree</span>
                <br>
                <button onclick="updateServo('gripper', 0)">Close</button>
                <button onclick="updateServo('gripper', 90)">Open</button>
            </div>
        </div>
        
        <div class="quick-moves">
            <h3>Quick Positions</h3>
            <button onclick="moveToPreset('home')">Home</button>
            <button onclick="moveToPreset('pickup')">Pick Up</button>
            <button onclick="moveToPreset('raised')">Raised</button>
            <button onclick="moveToPreset('reset')">Reset</button>
        </div>
    </div>

    <script>
        function updateServo(joint, angle) {
            fetch(`/servo?joint=${joint}&angle=${angle}`)
                .then(response => response.text())
                .then(data => {
                    document.getElementById(joint + 'Value').textContent = angle;
                    document.getElementById(joint + 'Slider').value = angle;
                });
        }
        
        function moveToPreset(preset) {
            fetch(`/preset?name=${preset}`)
                .then(response => response.text())
                .then(updateAllSliders);
        }
        
        function updateAllSliders() {
            fetch('/positions')
                .then(response => response.json())
                .then(positions => {
                    document.getElementById('baseSlider').value = positions.base;
                    document.getElementById('baseValue').textContent = positions.base;
                    document.getElementById('shoulderSlider').value = positions.shoulder;
                    document.getElementById('shoulderValue').textContent = positions.shoulder;
                    document.getElementById('elbowSlider').value = positions.elbow;
                    document.getElementById('elbowValue').textContent = positions.elbow;
                    document.getElementById('gripperSlider').value = positions.gripper;
                    document.getElementById('gripperValue').textContent = positions.gripper;
                });
        }
        
        // Update all sliders on page load
        window.onload = updateAllSliders;
    </script>
</body>
</html>
)rawliteral";

void setupCamera() {
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
    
    // Frame size configuration
    config.frame_size = FRAMESIZE_QVGA;  // 320x240
    config.jpeg_quality = 12;  // 0-63, lower = higher quality
    config.fb_count = 1;
    
    // Initialize camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }
    Serial.println("Camera initialized successfully");
}

void updateAllServos() {
    servoBase.write(baseAngle);
    servoShoulder.write(shoulderAngle);
    servoElbow.write(elbowAngle);
    servoGripper.write(gripperAngle);
}

void moveToHome() {
    baseAngle = 90;
    shoulderAngle = 90;
    elbowAngle = 90;
    gripperAngle = 90;
    
    updateAllServos();
}

void setupServos() {
    // Allow allocation of timers
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);
    
    servoBase.attach(SERVO_BASE_PIN);
    servoShoulder.attach(SERVO_SHOULDER_PIN);
    servoElbow.attach(SERVO_ELBOW_PIN);
    servoGripper.attach(SERVO_GRIPPER_PIN);
    
    // Move to default position
    moveToHome();
}

void handleServoControl() {
    String joint = server.arg("joint");
    int angle = server.arg("angle").toInt();
    
    // Constrain angle values
    angle = constrain(angle, 0, 180);
    
    if (joint == "base") {
        baseAngle = angle;
        servoBase.write(baseAngle);
    } else if (joint == "shoulder") {
        shoulderAngle = angle;
        servoShoulder.write(shoulderAngle);
    } else if (joint == "elbow") {
        elbowAngle = angle;
        servoElbow.write(elbowAngle);
    } else if (joint == "gripper") {
        gripperAngle = constrain(angle, 0, 90);  // Limit gripper range
        servoGripper.write(gripperAngle);
    }
    
    server.send(200, "text/plain", "OK");
}

void handlePreset() {
    String preset = server.arg("name");
    
    if (preset == "home") {
        moveToHome();
    } else if (preset == "pickup") {
        baseAngle = 90;
        shoulderAngle = 45;
        elbowAngle = 135;
        gripperAngle = 90;
        updateAllServos();
        delay(500);
        gripperAngle = 0;
        servoGripper.write(gripperAngle);
    } else if (preset == "raised") {
        baseAngle = 90;
        shoulderAngle = 45;
        elbowAngle = 45;
        gripperAngle = 90;
        updateAllServos();
    } else if (preset == "reset") {
        moveToHome();
    }
    
    server.send(200, "text/plain", "OK");
}

void handleGetPositions() {
    String json = "{";
    json += "\"base\":" + String(baseAngle) + ",";
    json += "\"shoulder\":" + String(shoulderAngle) + ",";
    json += "\"elbow\":" + String(elbowAngle) + ",";
    json += "\"gripper\":" + String(gripperAngle);
    json += "}";
    
    server.send(200, "application/json", json);
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n\n=================================");
    Serial.println("ESP32-CAM 4-DOF Robot Arm");
    Serial.println("=================================");
    
    // Initialize WiFi in AP mode
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    
    Serial.printf("WiFi AP Started\nSSID: %s\nPassword: %s\n", ssid, password);
    Serial.printf("IP Address: %s\n", WiFi.softAPIP().toString().c_str());
    
    // Initialize camera
    setupCamera();
    
    // Initialize servos
    setupServos();
    
    // Setup web server routes
    server.on("/", HTTP_GET, []() {
        server.send(200, "text/html", index_html);
    });
    
    server.on("/servo", HTTP_GET, handleServoControl);
    server.on("/preset", HTTP_GET, handlePreset);
    server.on("/positions", HTTP_GET, handleGetPositions);
    
    // Start streaming server
    server.on("/stream", HTTP_GET, []() {
        WiFiClient client = server.client();
        String response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
        server.sendContent(response);
        
        while (client.connected()) {
            camera_fb_t *fb = esp_camera_fb_get();
            if (!fb) {
                Serial.println("Camera capture failed");
                break;
            }
            
            response = "--frame\r\n";
            response += "Content-Type: image/jpeg\r\n\r\n";
            server.sendContent(response);
            
            client.write(fb->buf, fb->len);
            server.sendContent("\r\n");
            
            esp_camera_fb_return(fb);
            
            if (!client.connected()) break;
        }
    });
    
    server.begin();
    Serial.println("HTTP server started");
    Serial.println("=================================");
    Serial.println("Access the robot arm controller at:");
    Serial.printf("http://%s\n", WiFi.softAPIP().toString().c_str());
}

void loop() {
    server.handleClient();
    delay(2);
}
