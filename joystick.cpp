#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char* ssid = "Phantom Roulette";
const char* password = "12345678";

// PWM configuration
const int bldcPin = 18;  // GPIO pin for BLDC control
const int pwmFreq = 5000;  // PWM frequency
const int pwmResolution = 14;
const int bldcChannel = 10;
int strobeFrequency = 60;

// Joystick control variables
int joystickX = 0;
int joystickY = 0;

int bldcSpeed = 0;
AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, 16, 17); // Initialize Serial1 for communication with Arduino (TX=17, RX=16) at 115200 baud rate
  // Configure PWM channels
  ledcSetup(bldcChannel, pwmFreq, pwmResolution);
  ledcAttachPin(bldcPin, bldcChannel);
  Serial1.println(strobeFrequency);
  WiFi.softAP(ssid, password);

  // Define web server routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = R"rawliteral(
    <!DOCTYPE html>
    <html lang="zh-Hant-TW">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no, viewport-fit=cover">
        <meta name="apple-mobile-web-app-capable" content="yes">
        <meta name="apple-mobile-web-app-status-bar-style" content="black-translucent">
        <meta name="format-detection" content="telephone=no">
        <title>Phantom Roulette v1.16.0</title>
        <style>
            * {
                -webkit-tap-highlight-color: transparent;
                -webkit-touch-callout: none;
                -webkit-user-select: none;
                user-select: none;
                box-sizing: border-box;
            }

            input {
                -webkit-user-select: auto;
                user-select: auto;
            }

            body {
                font-family: -apple-system, system-ui, BlinkMacSystemFont, "Segoe UI", Roboto, Arial, sans-serif;
                background-color: #0d1117;
                color: #c9d1d9;
                margin: 0;
                padding: 20px;
                padding-top: env(safe-area-inset-top);
                padding-bottom: env(safe-area-inset-bottom);
                padding-left: env(safe-area-inset-left);
                padding-right: env(safe-area-inset-right);
                display: flex;
                justify-content: center;
                align-items: center;
                min-height: 100vh;
            }

            .container {
                background-color: #161b22;
                border-radius: 16px;
                padding: 20px;
                width: 100%;
                max-width: 400px;
                box-shadow: 0 1px 3px rgba(27, 31, 35, 0.12), 0 8px 24px rgba(27, 31, 35, 0.12);
            }

            h2 {
                text-align: center;
                color: #c9d1d9;
                margin-bottom: 5px;
                font-size: 24px;
            }

            p {
                text-align: center;
                color: #8b949e;
                margin-top: 0;
                margin-bottom: 20px;
                font-size: 16px;
            }

            .control-group {
                margin-bottom: 24px;
            }

            label {
                display: block;
                margin-bottom: 8px;
                color: #c9d1d9;
                font-size: 16px;
            }

            .input-group {
                display: flex;
                align-items: center;
                margin-bottom: 12px;
                position: relative;
            }

            input[type="text"] {
                flex-grow: 1;
                background-color: #0d1117;
                border: 1px solid #30363d;
                color: #c9d1d9;
                padding: 12px;
                border-radius: 8px;
                font-size: 16px;
                margin-right: 8px;
                width: 100%;
            }

            .unit {
                position: absolute;
                right: 12px;
                color: #8b949e;
                pointer-events: none;
            }

            .button-group {
                display: grid;
                grid-template-columns: repeat(auto-fit, minmax(70px, 1fr));
                gap: 8px;
                margin-top: 12px;
            }

            .multiplier-group {
                display: grid;
                grid-template-columns: repeat(auto-fit, minmax(70px, 1fr));
                align-items: center;
                gap: 8px;
                margin-top: 12px;
            }

            #customMultiplier {
                flex-grow: 1;
                max-width: 80px;
                text-align: center;
                background-color: #0d1117;
                border: 1px solid #30363d;
                color: #c9d1d9;
                padding: 12px;
                border-radius: 8px;
                font-size: 16px;
            }

            button {
                background-color: #238636;
                color: #ffffff;
                border: none;
                padding: 12px;
                border-radius: 8px;
                cursor: pointer;
                font-size: 16px;
                font-weight: 500;
                transition: background-color 0.2s ease;
                min-height: 44px;
            }

            button:active {
                background-color: #2ea043;
                transform: scale(0.98);
            }

            .error-message {
                color: #f85149;
                font-size: 14px;
                margin-top: 4px;
                display: none;
            }

            /* Joystick Styles */
            #joystick-container {
                width: 200px;
                height: 200px;
                background-color: #30363d;
                border-radius: 50%;
                position: relative;
                margin: 0 auto;
                touch-action: none;
            }

            #joystick {
                width: 50px;
                height: 50px;
                background-color: #58a6ff;
                border-radius: 50%;
                position: absolute;
                top: 50%;
                left: 50%;
                transform: translate(-50%, -50%);
                cursor: grab;
            }

            #joystick:active {
                cursor: grabbing;
            }

            #joystick-coordinates {
                text-align: center;
                margin-top: 10px;
                color: #8b949e;
            }
        </style>
    </head>
    <body>
        <div class="container">
            <h2>幻影輪盤</h2>
            <p>Phantom Roulette</p>
            
            <!-- Previous controls remain the same -->

            <div class="control-group">
                <label>遙控搖桿 Joystick Control:</label>
                <div id="joystick-container">
                    <div id="joystick"></div>
                </div>
                <div id="joystick-coordinates">X: 0, Y: 0</div>
            </div>
        </div>

        <script>
            // Previous JavaScript remains the same

            // Joystick Control
            const joystickContainer = document.getElementById('joystick-container');
            const joystick = document.getElementById('joystick');
            const joystickCoordinates = document.getElementById('joystick-coordinates');
            let isDragging = false;
            let startX, startY;
            const maxDistance = 100;

            function updateJoystickPosition(clientX, clientY) {
                const rect = joystickContainer.getBoundingClientRect();
                const centerX = rect.left + rect.width / 2;
                const centerY = rect.top + rect.height / 2;

                let deltaX = clientX - centerX;
                let deltaY = clientY - centerY;

                const distance = Math.sqrt(deltaX * deltaX + deltaY * deltaY);
                
                if (distance > maxDistance) {
                    const angle = Math.atan2(deltaY, deltaX);
                    deltaX = Math.cos(angle) * maxDistance;
                    deltaY = Math.sin(angle) * maxDistance;
                }

                joystick.style.transform = `translate(${deltaX}px, ${deltaY}px)`;
                
                // Normalize coordinates to -100 to 100 range
                const normX = Math.round(deltaX / maxDistance * 100);
                const normY = Math.round(-deltaY / maxDistance * 100);

                joystickCoordinates.textContent = `X: ${normX}, Y: ${normY}`;

                // Send joystick coordinates to server
                const xhr = new XMLHttpRequest();
                xhr.open('GET', `/setJoystick?x=${normX}&y=${normY}`, true);
                xhr.send();
            }

            function startDragging(e) {
                isDragging = true;
                const clientX = e.type.includes('mouse') ? e.clientX : e.touches[0].clientX;
                const clientY = e.type.includes('mouse') ? e.clientY : e.touches[0].clientY;
                
                updateJoystickPosition(clientX, clientY);
            }

            function stopDragging() {
                if (isDragging) {
                    isDragging = false;
                    joystick.style.transform = 'translate(-50%, -50%)';
                    joystickCoordinates.textContent = 'X: 0, Y: 0';
                    
                    // Reset joystick to center
                    const xhr = new XMLHttpRequest();
                    xhr.open('GET', `/setJoystick?x=0&y=0`, true);
                    xhr.send();
                }
            }

            function drag(e) {
                if (!isDragging) return;
                
                const clientX = e.type.includes('mouse') ? e.clientX : e.touches[0].clientX;
                const clientY = e.type.includes('mouse') ? e.clientY : e.touches[0].clientY;
                
                updateJoystickPosition(clientX, clientY);
            }

            // Mouse events
            joystick.addEventListener('mousedown', startDragging);
            document.addEventListener('mousemove', drag);
            document.addEventListener('mouseup', stopDragging);

            // Touch events
            joystick.addEventListener('touchstart', startDragging);
            document.addEventListener('touchmove', drag);
            document.addEventListener('touchend', stopDragging);
        </script>
    </body>
    </html>
    )rawliteral";
    request->send(200, "text/html", html);
  });
  
  // Previous routes remain the same

  // New route for joystick control
  server.on("/setJoystick", HTTP_GET, [](AsyncWebServerRequest *request){
    int x = request->getParam("x")->value().toInt();
    int y = request->getParam("y")->value().toInt();
    
    // Normalize and constrain values
    joystickX = constrain(x, -100, 100);
    joystickY = constrain(y, -100, 100);
    
    // Optional: Map joystick values to specific actions
    // For example, use Y-axis to control motor speed, X-axis for something else
    int mappedSpeed = map(abs(joystickY), 0, 100, 0, 8096);
    bldcSpeed = joystickY > 0 ? mappedSpeed : 0;
    ledcWrite(bldcChannel, bldcSpeed);
    
    request->send(200, "text/plain", "Joystick: X=" + String(joystickX) + ", Y=" + String(joystickY));
  });

  server.begin();
}

void loop() {
  // Potential future use for additional processing
}