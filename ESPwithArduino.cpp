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
  //Serial.println("Access Point Started");
  //Serial.print("IP Address: ");
  //Serial.println(WiFi.softAPIP());
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
        <title>Phantom Roulette v1.15.2</title>
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
        </style>
    </head>
    <body>
        <div class="container">
            <h2>幻影輪盤</h2>
            <p>Phantom Roulette</p>
            
            <div class="control-group">
                <label for="bldc">無刷馬達速度 Motor Speed(PWM):</label>
                <div class="input-group">
                    <input type="text" id="bldc" inputmode="decimal" placeholder="4.800"
                        aria-label="Motor Speed PWM">
                    <span class="unit">%</span>
                    <button ontouchstart="updatePWM('bldc', 4.8)">Reset</button>
                </div>
                <div id="bldcError" class="error-message">請輸入 5-40 之間的數值 (Please enter a value between 5-40)</div>
                <div class="button-group">
                    <button ontouchstart="updatePWM('bldc', parseFloat(document.getElementById('bldc').value) - 0.01)">-0.01%</button>
                    <button ontouchstart="updatePWM('bldc', parseFloat(document.getElementById('bldc').value) - 0.1)">-0.1%</button>
                    <button ontouchstart="updatePWM('bldc', parseFloat(document.getElementById('bldc').value) + 0.1)">+0.1%</button>
                    <button ontouchstart="updatePWM('bldc', parseFloat(document.getElementById('bldc').value) + 0.01)">+0.01%</button>
                </div>
            </div>
            
            <div class="control-group">
                <label for="freq">閃爍頻率 Strobe Frequency:</label>
                <div class="input-group">
                    <input type="text" id="freq" inputmode="numeric" placeholder="60"
                        aria-label="Strobe Frequency">
                    <span class="unit">Hz</span>
                    <button ontouchstart="updateFreq(60)">Reset</button>
                </div>
                <div id="freqError" class="error-message">請輸入 30-480 之間的數值 (Please enter a value between 30-480)</div>
                
                <div class="multiplier-group">
                    <input type="text" id="customMultiplier" inputmode="decimal" placeholder="2.0"
                        aria-label="Multiplier">
                    <button ontouchstart="divideFreq()">Divide</button>
                    <button ontouchstart="multiplyFreq()">Multiply</button>
                    <button ontouchstart="resetMultiplier()">Reset</button>
                </div>
                
                <div class="button-group">
                    <button ontouchstart="updateFreq(50)">50 Hz</button>
                    <button ontouchstart="updateFreq(60)">60 Hz</button>
                    <button ontouchstart="updateFreq(parseFloat(document.getElementById('freq').value) - 1)">-1 Hz</button>
                    <button ontouchstart="updateFreq(parseFloat(document.getElementById('freq').value) + 1)">+1 Hz</button>
                </div>
            </div>
        </div>

        <script>
            // Initialize default values
            window.onload = function() {
                document.getElementById('bldc').value = '4.800';
                document.getElementById('freq').value = '60';
                document.getElementById('customMultiplier').value = '2.0';
            };

            function validateNumber(value, min, max, decimals = 0) {
                // Remove any non-numeric characters except decimal point
                value = value.replace(/[^\d.]/g, '');
                
                // Ensure only one decimal point
                const parts = value.split('.');
                if (parts.length > 2) {
                    value = parts[0] + '.' + parts.slice(1).join('');
                }
                
                // Convert to number and check bounds
                const num = parseFloat(value);
                if (isNaN(num)) return null;
                
                // Round to specified decimal places
                const multiplier = Math.pow(10, decimals);
                return Math.min(max, Math.max(min, Math.round(num * multiplier) / multiplier));
            }

            function updatePWM(device, value) {
                const input = document.getElementById(device);
                const errorElement = document.getElementById(device + 'Error');
                
                const validatedValue = validateNumber(value.toString(), 5, 40, 3);
                
                if (validatedValue === null) {
                    errorElement.style.display = 'block';
                    return;
                } 
                
                errorElement.style.display = 'none';
                input.value = validatedValue.toFixed(3);
                
                const pwmValue = Math.round((validatedValue / 100) * 16383);
                const xhr = new XMLHttpRequest();
                xhr.open('GET', '/setPWM?device=' + device + '&value=' + pwmValue, true);
                xhr.send();
            }

            function updateFreq(value) {
                const input = document.getElementById('freq');
                const errorElement = document.getElementById('freqError');
                
                const validatedValue = validateNumber(value.toString(), 30, 480, 0);
                
                if (validatedValue === null) {
                    errorElement.style.display = 'block';
                    return;
                }
                
                errorElement.style.display = 'none';
                input.value = validatedValue.toString();
                
                const xhr = new XMLHttpRequest();
                xhr.open('GET', '/setFreq?value=' + validatedValue, true);
                xhr.send();
            }

            function multiplyFreq() {
                const freq = parseFloat(document.getElementById('freq').value);
                const multiplier = parseFloat(document.getElementById('customMultiplier').value);
                if (!isNaN(freq) && !isNaN(multiplier)) {
                    updateFreq(freq * multiplier);
                }
            }

            function divideFreq() {
                const freq = parseFloat(document.getElementById('freq').value);
                const multiplier = parseFloat(document.getElementById('customMultiplier').value);
                if (!isNaN(freq) && !isNaN(multiplier) && multiplier !== 0) {
                    updateFreq(freq / multiplier);
                }
            }
            function resetMultiplier() {
                document.getElementById('customMultiplier').value = '2.0';
            }

            // Add input event listeners
            document.getElementById('bldc').addEventListener('input', function(e) {
                updatePWM('bldc', e.target.value);
            });

            document.getElementById('freq').addEventListener('input', function(e) {
                updateFreq(e.target.value);
            });

            document.getElementById('customMultiplier').addEventListener('input', function(e) {
                const value = validateNumber(e.target.value, 1, 10, 1);
                if (value !== null) {
                    e.target.value = value.toFixed(1);
                }
            });
        </script>
    </body>
    </html>
    )rawliteral";
    request->send(200, "text/html", html);
  });
  
  
  server.on("/setPWM", HTTP_GET, [](AsyncWebServerRequest *request){
    String device = request->getParam("device")->value();
    int value = request->getParam("value")->value().toInt();
    
    if (device == "bldc") {
      if (value!=0 && value!=4) bldcSpeed = constrain(value, 4, 8096);
      else bldcSpeed = 0;
      ledcWrite(bldcChannel, bldcSpeed);
    }    
    request->send(200, "text/plain", "OK");
  });

  server.on("/setFreq", HTTP_GET, [](AsyncWebServerRequest *request){
    int value = request->getParam("value")->value().toInt();
    strobeFrequency = constrain(value, 30, 480);
    Serial1.println(strobeFrequency); // Send strobe frequency to Arduino
    request->send(200, "text/plain", "OK");
  });
  server.begin();
}

void loop() {
  
  
}