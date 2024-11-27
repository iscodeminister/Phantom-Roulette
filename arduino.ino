const int pwmPin = 9;   // PWM output pin (using Timer1)

String message = "";    // Initialize message as a global variable
unsigned long defaultFrequency = 60;  // Default frequency if no message received

void setup() {
  Serial.begin(115200);
  pinMode(pwmPin, OUTPUT);  // Set PWM pin as output
  
  // Configure Timer1 for Phase and Frequency Correct PWM mode
  noInterrupts();  // Disable interrupts while configuring
  
  // Clear control registers
  TCCR1A = 0;
  TCCR1B = 0;
  
  // Set mode to Phase and Frequency Correct PWM, ICR1 as TOP
  TCCR1A = (1 << COM1A1);  // Clear OC1A on match when upcounting
  TCCR1B = (1 << WGM13) | (1 << CS11);  // Prescaler set to 8
  
  interrupts();  // Re-enable interrupts
}

void loop() {
  // Check for serial message
  if (Serial.available()) {
    message = Serial.readStringUntil('\n');  // Read until newline
    message.trim();  // Remove any whitespace
  }
  
  // Convert message to frequency, use default if conversion fails
  unsigned long frequency = message.length() > 0 ? message.toInt() : defaultFrequency;
  
  // Constrain frequency to a reasonable range
  frequency = constrain(frequency, 40, 480);
  
  // Calculate TOP value for the given frequency
  unsigned int topValue = (16000000UL / (2UL * 8UL * frequency)) - 1;
  
  // Disable interrupts while updating registers
  noInterrupts();
  ICR1 = topValue;  // Update TOP value to adjust frequency
  OCR1A = topValue * 0.8 / 100;  // 5% duty cycle
  interrupts();
  
  // Optional: Add a small delay or use millis() for non-blocking behavior
  delay(10);
}