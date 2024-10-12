#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char* ssid = "Phantom Roulette";
const char* password = "optic2024";

// PWM configuration
const int bldcPin = 26;  // GPIO pin for BLDC control
const int strobePin = 27;  // GPIO pin for strobe light
const int pwmFreq = 250;  // 5 kHz PWM frequency
const int pwmResolution = 14;
const int bldcChannel = 10;
const int strobeChannel = 0;

int bldcSpeed = 0;
int strobeIntensity = 8192;
float strobeFrequency = 60; 
AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);
  
  //ledcSetClockSource(LEDC_REF_CLK source);
  // Configure PWM channels
  ledcSetup(bldcChannel, pwmFreq, pwmResolution);
  ledcSetup(strobeChannel, pwmFreq, pwmResolution);
  ledcAttachPin(bldcPin, bldcChannel);
  ledcAttachPin(strobePin, strobeChannel);
  
  WiFi.softAP(ssid, password);
  Serial.println("Access Point Started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());
  // Define web server routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = R"rawliteral(
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, user-scalable=no, maximum-scale=1" />
        <title>Phantom Roulette v1.14</title>
        <style>
            body {
                font-family: Arial, sans-serif;
                background-color: #0d1117;
                color: #c9d1d9;
                margin: 0;
                padding: 20px;
                display: flex;
                justify-content: center;
                align-items: center;
                min-height: 100vh;
            }
            .container {
                background-color: #161b22;
                border-radius: 10px;
                padding: 20px;
                width: 100%;
                max-width: 400px;
                box-shadow: 0 1px 3px rgba(27, 31, 35, 0.12), 0 8px 24px rgba(27, 31, 35, 0.12);
            }
            h2 {
                text-align: center;
                color: #c9d1d9;
                margin-bottom: 5px;
            }
            p {
                text-align: center;
                color: #8b949e;
                margin-top: 0;
                margin-bottom: 20px;
            }
            .control-group {
                margin-bottom: 20px;
            }
            label {
                display: block;
                margin-bottom: 5px;
                color: #c9d1d9;
            }
            .input-group {
                display: flex;
                align-items: center;
                margin-bottom: 10px;
            }
            input[type="number"] {
                flex-grow: 1;
                background-color: #0d1117;
                border: 1px solid #30363d;
                color: #c9d1d9;
                padding: 10px;
                border-radius: 5px;
            }
            .value-display {
                margin-left: 10px;
                color: #c9d1d9;
                font-weight: bold;
            }
            .button-group {
                display: flex;
                justify-content: space-between;
            }
            button {
                background-color: #238636;
                color: #ffffff;
                border: none;
                padding: 10px 20px;
                border-radius: 5px;
                cursor: pointer;
            }
            button:hover {
                background-color: #2ea043;
            }
        </style>
    </head>
    <body>
        <div class="container">
            <h2>Phantom Roulette</h2>
            <p>Try it yourself</p>
            
            <div class="control-group">
                <label for="bldc">Brushless Motor Speed:</label>
                <div class="input-group">
                    <input type="number" id="bldc" min="5" max="50" value="10.000" step="0.001" onchange="updatePWM('bldc', this.value)">
                    <span>%</span>
                </div>
                <div class="button-group">
                    <button onclick="updatePWM('bldc', parseFloat(document.getElementById('bldc').value) - 0.001)">-0.001%</button>
                    <button onclick="updatePWM('bldc', parseFloat(document.getElementById('bldc').value) - 0.01)">-0.01%</button>
                    <button onclick="updatePWM('bldc', parseFloat(document.getElementById('bldc').value) + 0.01)">+0.01%</button>
                    <button onclick="updatePWM('bldc', parseFloat(document.getElementById('bldc').value) + 0.001)">+0.001%</button>
                    <button onclick="updatePWM('bldc', 0)">Reset</button>
                </div>
            </div>
            
            <div class="control-group">
                <label for="freq">Strobe Frequency (Hz):</label>
                <div class="input-group">
                    <input type="number" id="freq" min="30" max="480" value="60" onchange="updateFreq(this.value)">
                    <span>Hz</span>
                </div>
                <div class="button-group">
                    <button onclick="updateFreq(parseFloat(document.getElementById('freq').value) / 2)">Divide 2</button>
                    <button onclick="updateFreq(parseFloat(document.getElementById('freq').value) * 2)">Multiply 2</button>
                    <button onclick="updateFreq(60)">Reset</button>
                </div>
            </div>
        </div>

        <script>
            function updatePWM(device, value) {
                value = Math.max(0, Math.min(100, parseFloat(value))).toFixed(2);
                document.getElementById(device).value = value;
                var pwmValue = Math.round((value / 100) * 16383);
                var xhr = new XMLHttpRequest();
                xhr.open('GET', '/setPWM?device=' + device + '&value=' + pwmValue, true);
                xhr.send();
            }

            function updateFreq(value) {
                value = Math.max(30, Math.min(480, Math.round(parseFloat(value))));
                document.getElementById('freq').value = value;
                var xhr = new XMLHttpRequest();
                xhr.open('GET', '/setFreq?value=' + value, true);
                xhr.send();
            }
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
      if (value!=0) bldcSpeed = constrain(value, 800, 8000);
      else bldcSpeed = 0;
      ledcWrite(bldcChannel, bldcSpeed);
    }    
    request->send(200, "text/plain", "OK");
  });

  server.on("/setFreq", HTTP_GET, [](AsyncWebServerRequest *request){
    int value = request->getParam("value")->value().toInt();
    strobeFrequency = constrain(value, 30, 480);
    ledcSetup(strobeChannel, strobeFrequency, pwmResolution);
    ledcWrite(strobeChannel, strobeIntensity);  // Reapply intensity after changing frequency
    request->send(200, "text/plain", "OK");
  });
  ledcWrite(strobeChannel, strobeIntensity);
  server.begin();
}

void loop() {
  
  
}